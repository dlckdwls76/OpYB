// 에픽게임즈 저작권 소유.


#include "TwinStickCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "TwinStickGameMode.h"
#include "TwinStickAoEAttack.h"
#include "Kismet/KismetMathLibrary.h"
#include "TwinStickProjectile.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATwinStickCharacter::ATwinStickCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

	// 스프링 암 생성
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);

	SpringArm->SetRelativeRotation(FRotator(-50.0f, 0.0f, 0.0f));

	SpringArm->TargetArmLength = 2200.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 0.5f;

	// 카메라 생성
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	Camera->SetFieldOfView(75.0f);

	// 캐릭터 무브먼트 설정
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
}

void ATwinStickCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 아이템 개수 업데이트
	UpdateItems();
}

void ATwinStickCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	/** Clear the autofire timer */
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void ATwinStickCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// 플레이어 컨트롤러 레퍼런스 설정
	PlayerController = Cast<APlayerController>(GetController());
}

void ATwinStickCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 현재 회전 가져오기
	const FRotator OldRotation = GetActorRotation();

	// 마우스로 조준 중입니까?
	if (bUsingMouse)
	{
		if (PlayerController)
		{
			// 커서의 월드 위치 가져오기
			FHitResult OutHit; 
			PlayerController->GetHitResultUnderCursorByChannel(MouseAimTraceChannel, true, OutHit);

			// 조준 회전 찾기 
			const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), OutHit.Location);

			// 조준 각도 저장
			AimAngle = AimRot.Yaw;

			

			// 요(Yaw)를 업데이트하고 피치(Pitch)와 롤(Roll) 재사용
			SetActorRotation(FRotator(OldRotation.Pitch, AimAngle, OldRotation.Roll));

		}

	} else {

		// 쿼터니언 보간을 사용하여 현재 회전 간 부드럽게 블렌딩
		// 최단 경로를 사용한 목표 조준 회전
		const FRotator TargetRot = FRotator(OldRotation.Pitch, AimAngle, OldRotation.Roll);

		SetActorRotation(TargetRot);
	}
}

void ATwinStickCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 향상된 입력 액션 바인딩 설정
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::Move);
		EnhancedInputComponent->BindAction(StickAimAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::StickAim);
		EnhancedInputComponent->BindAction(MouseAimAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::MouseAim);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::Dash);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::Shoot);
		EnhancedInputComponent->BindAction(AoEAction, ETriggerEvent::Triggered, this, &ATwinStickCharacter::AoEAttack);

	}

}

void ATwinStickCharacter::Move(const FInputActionValue& Value)
{
	// 입력 저장 vector
	FVector2D InputVector = Value.Get<FVector2D>();

	// 입력 전달
	DoMove(InputVector.X, InputVector.Y);
}

void ATwinStickCharacter::StickAim(const FInputActionValue& Value)
{
	// 입력 벡터 가져오기
	FVector2D InputVector = Value.Get<FVector2D>();

	// 입력 전달
	DoAim(InputVector.X, InputVector.Y);
}

void ATwinStickCharacter::MouseAim(const FInputActionValue& Value)
{
	// 마우스 컨트롤 플래그 올리기
	bUsingMouse = true;

	// 마우스 커서 표시
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(true);
	}
}

void ATwinStickCharacter::Dash(const FInputActionValue& Value)
{
	// 입력 전달
	DoDash();
}

void ATwinStickCharacter::Shoot(const FInputActionValue& Value)
{
	// 입력 전달
	DoShoot();
}

void ATwinStickCharacter::AoEAttack(const FInputActionValue& Value)
{
	// 입력 전달
	DoAoEAttack();
}

void ATwinStickCharacter::DoMove(float AxisX, float AxisY)
{
	// 입력 저장
	LastMoveInput.X = AxisX;
	LastMoveInput.Y = AxisY;

	// 입력의 앞으로 이동 성분 계산
	FRotator FlatRot = GetControlRotation();
	FlatRot.Pitch = 0.0f;

	// 앞으로 이동 입력 적용
	AddMovementInput(FlatRot.RotateVector(FVector::ForwardVector), AxisX);

	// 오른쪽 이동 입력 적용
	AddMovementInput(FlatRot.RotateVector(FVector::RightVector), AxisY);
}

void ATwinStickCharacter::DoAim(float AxisX, float AxisY)
{
	// 입력에서 조준 각도 계산
	AimAngle = FMath::RadiansToDegrees(FMath::Atan2(AxisY, -AxisX));

	// 마우스 컨트롤 플래그 해제
	bUsingMouse = false;

	// 마우스 커서 숨기기
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(false);
	}

	// 자동 발사 쿨타임 중입니까?
	if (!bAutoFireActive)
	{
		// 자신을 쿨타임 상태로 설정
		bAutoFireActive = true;

		// 투사체 발사
		DoShoot();

		// 자동 발사 쿨타임 재설정 예약
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &ATwinStickCharacter::ResetAutoFire, AutoFireDelay, false);
	}
}

void ATwinStickCharacter::DoDash()
{
	// 마지막 이동 입력에 기반한 발사 임펄스 벡터 계산
	FVector LaunchDir = FVector::ZeroVector;

	LaunchDir.X = FMath::Clamp(LastMoveInput.X, -1.0f, 1.0f);
	LaunchDir.Y = FMath::Clamp(LastMoveInput.Y, -1.0f, 1.0f);

	// 선택한 방향으로 캐릭터 발사
	LaunchCharacter(LaunchDir * DashImpulse, true, true);

	// 구르기 몽타주 재생
	if (RollMontage)
	{
		PlayAnimMontage(RollMontage);
	}
}

void ATwinStickCharacter::DoShoot()
{
	// 액터 트랜스폼 가져오기
	FTransform ProjectileTransform = GetActorTransform();

	// 투사체 스폰 오프셋 적용
	FVector ProjectileLocation = ProjectileTransform.GetLocation() + ProjectileTransform.GetRotation().RotateVector(FVector::ForwardVector * ProjectileOffset);
	ProjectileTransform.SetLocation(ProjectileLocation);

	ATwinStickProjectile* Projectile = GetWorld()->SpawnActor<ATwinStickProjectile>(ProjectileClass, ProjectileTransform);
}

void ATwinStickCharacter::DoAoEAttack()
{
	// 광역 공격을 수행할 충분한 아이템이 있습니까?
	if (Items > 0)
	{
		// 게임 시간 가져오기
		const float GameTime = GetWorld()->GetTimeSeconds();

		// 광역 공격 쿨타임이 끝났습니까?
		if (GameTime - LastAoETime > AoECooldownTime)
		{
			// 새로운 광역 공격 시간 저장
			LastAoETime = GameTime;

			// 광역 공격 스폰
			ATwinStickAoEAttack* AoE = GetWorld()->SpawnActor<ATwinStickAoEAttack>(AoEAttackClass, GetActorTransform());

			// 아이템 수 감소
			--Items;

			// 아이템 개수 업데이트
			UpdateItems();
		}
	}
}

void ATwinStickCharacter::HandleDamage(float Damage, const FVector& DamageDirection)
{
	// 넉백 벡터 계산
	FVector LaunchVector = DamageDirection;
	LaunchVector.Z = 0.0f;

	// 캐릭터에게 넉백 적용
	LaunchCharacter(LaunchVector * KnockbackStrength, true, true);

	// BP로 제어권 넘기기
	BP_Damaged();
}

void ATwinStickCharacter::AddPickup()
{
	// 아이템 카운트 증가
	++Items;

	// 아이템 개수 업데이트er
	UpdateItems();
}

void ATwinStickCharacter::UpdateItems()
{
	// 게임 모드 업데이트
	if (ATwinStickGameMode* GM = Cast<ATwinStickGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ItemUsed(Items);
	}
}

void ATwinStickCharacter::ResetAutoFire()
{
	// 자동 발사 플래그 초기화
	bAutoFireActive = false;
}

