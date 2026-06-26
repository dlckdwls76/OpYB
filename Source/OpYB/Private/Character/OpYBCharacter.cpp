// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/OpYBCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/OpYBOverheadWidget.h"
#include "UI/OpYBDeathScreenWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Actor/OpYBProjectile.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"

AOpYBCharacter::AOpYBCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // 마우스 커서를 바라봐야 하므로 이동 방향 회전 끔
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true; // 구르기나 점프 등을 위해 평면 제약 해제
	GetCharacterMovement()->bSnapToPlaneAtStart = false;

	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Combat Defaults
	FireRate = 0.1f; // 0.1초마다 1발 (단발 사격 시 답답함 방지)
	LastFireTime = 0.0f;

	// Ammo Defaults
	MaxAmmo = 3;
	CurrentAmmo = MaxAmmo;
	ReloadTimePerAmmo = 3.0f; // 1초당 1발 충전

	// Overhead UI Component
	OverheadUIComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadUI"));
	OverheadUIComponent->SetupAttachment(RootComponent);
	OverheadUIComponent->SetWidgetSpace(EWidgetSpace::Screen); // 화면을 항상 바라보게 설정
	OverheadUIComponent->SetDrawSize(FVector2D(150.0f, 60.0f));
	OverheadUIComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // 캐릭터 머리 위로 올림
}

void AOpYBCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		CurrentAmmo = MaxAmmo;
	}
	
	LastAmmoCount = CurrentAmmo;
	LocalReloadProgress = 0.0f;
	bIsDead = false;
	RespawnTimeLeft = 0.0f;

	// 로컬 환경(서버/클라이언트 모두)에서 시작 시 UI 동기화를 위해 한 번 호출
	OnRep_CurrentHealth();
	OnRep_CurrentAmmo();
}

void AOpYBCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOpYBCharacter, CurrentHealth);
	DOREPLIFETIME(AOpYBCharacter, CurrentAmmo);
	DOREPLIFETIME(AOpYBCharacter, bIsDead);
}

void AOpYBCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		// 죽은 상태일 때 데스 스크린 UI의 남은 시간 갱신
		if (IsLocallyControlled() && DeathScreenInstance)
		{
			RespawnTimeLeft -= DeltaSeconds;
			if (RespawnTimeLeft < 0.0f) RespawnTimeLeft = 0.0f;
			DeathScreenInstance->UpdateTimeLeft(FMath::CeilToInt(RespawnTimeLeft));
		}
		return;
	}

	// 탄약이 꽉 차지 않았다면 로컬 장전 진행도 증가
	if (CurrentAmmo < MaxAmmo)
	{
		LocalReloadProgress += DeltaSeconds / ReloadTimePerAmmo;
		if (LocalReloadProgress > 1.0f)
		{
			LocalReloadProgress = 1.0f; // 서버의 실제 장전을 기다림
		}
	}
	else
	{
		LocalReloadProgress = 0.0f;
	}

	// 매 틱마다 UI 부드럽게 갱신
	if (OverheadUIComponent)
	{
		UOpYBOverheadWidget* OverheadWidget = Cast<UOpYBOverheadWidget>(OverheadUIComponent->GetUserWidgetObject());
		if (OverheadWidget)
		{
			OverheadWidget->UpdateAmmo(CurrentAmmo, MaxAmmo, LocalReloadProgress);
		}
	}
}

void AOpYBCharacter::PlayRollMontage()
{
	if (RollMontage && !IsRolling())
	{
		PlayAnimMontage(RollMontage);
	}
}

bool AOpYBCharacter::IsRolling() const
{
	if (GetMesh() && GetMesh()->GetAnimInstance() && RollMontage)
	{
		return GetMesh()->GetAnimInstance()->Montage_IsPlaying(RollMontage);
	}
	return false;
}

void AOpYBCharacter::AttemptShoot()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Warning, TEXT("2. [Character] AttemptShoot 호출됨! (쿨타임 및 탄약 검사 중...)"));

		// 탄약이 남아있고, 연사 쿨타임이 지났는지 로컬에서 1차 확인
	if (CurrentAmmo > 0 && CurrentTime - LastFireTime >= FireRate)
	{
		LastFireTime = CurrentTime;

		// 로컬 예측 (UI 바로 깎이게) - 단, 호스트(서버)는 어차피 바로 ServerShoot에서 깎이므로 이중 차감을 막기 위해 클라이언트일 때만 실행합니다.
		if (!HasAuthority())
		{
			CurrentAmmo--;
			OnRep_CurrentAmmo();
		}

		UE_LOG(LogTemp, Warning, TEXT("3. [Character] 로컬 발사 승인! 서버에 요청을 보냅니다. 남은 탄약: %d"), CurrentAmmo);

		// 총알 스폰 위치 및 회전 계산
		FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 20.0f);
		FRotator SpawnRotation = GetActorRotation();
		
		// ServerRPC 호출하여 서버에서 총알 생성 및 실제 탄약 소모 처리
		ServerShoot(SpawnLocation, SpawnRotation);
	}
	else if (CurrentAmmo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("X. [Character] 총알이 부족합니다! (재장전 중...)"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. [Character] 연사 쿨타임 대기 중!"));
	}
}

void AOpYBCharacter::RechargeAmmo()
{
	if (HasAuthority())
	{
		if (CurrentAmmo < MaxAmmo)
		{
			CurrentAmmo++;
			OnRep_CurrentAmmo(); // 서버 UI 갱신을 위해 수동 호출
			UE_LOG(LogTemp, Warning, TEXT("[Ammo Recharged] 1발 충전됨! 현재 탄약: %d / %d"), CurrentAmmo, MaxAmmo);
		}

		// 탄약이 꽉 찼다면 반복 타이머를 정지시킵니다.
		if (CurrentAmmo >= MaxAmmo)
		{
			GetWorldTimerManager().ClearTimer(RechargeTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("[Ammo Full] 탄약이 모두 충전되어 타이머를 정지합니다."));
		}
	}
}

bool AOpYBCharacter::ServerShoot_Validate(FVector SpawnLocation, FRotator SpawnRotation)
{
	return true;
}

void AOpYBCharacter::ServerShoot_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	UE_LOG(LogTemp, Warning, TEXT("4. [Server] 서버에서 스폰 요청 받음!"));

	if (ProjectileClass)
	{
		// Deferred Spawning (지연 생성)을 사용하여, 물리 충돌이 계산되기 전에 Owner와 Instigator를 완벽하게 주입합니다.
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		AOpYBProjectile* Proj = GetWorld()->SpawnActorDeferred<AOpYBProjectile>(ProjectileClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		if (Proj)
		{
			// 생성을 완료하며 물리 엔진에 등록
			Proj->FinishSpawning(SpawnTransform);
			UE_LOG(LogTemp, Warning, TEXT("5. [Server] 총알 최종 스폰 완료!"));
			// 총알 소모 처리 및 UI 업데이트 호출 (서버 권한)
			if (CurrentAmmo > 0)
			{
				CurrentAmmo--;
				OnRep_CurrentAmmo(); // 서버 측 로컬 UI 업데이트를 위해 수동 호출

				// 브롤스타즈 식 자동 충전 타이머 시작 (서버에서 관리)
				if (!GetWorldTimerManager().IsTimerActive(RechargeTimerHandle))
				{
					// bLoop = true 로 설정하여 꽉 찰 때까지 계속 반복해서 호출되도록 합니다.
					GetWorldTimerManager().SetTimer(RechargeTimerHandle, this, &AOpYBCharacter::RechargeAmmo, ReloadTimePerAmmo, true);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. [Server] ProjectileClass가 블루프린트에 등록되어 있지 않습니다!!"));
	}
}

void AOpYBCharacter::OnRep_CurrentHealth()
{
	if (OverheadUIComponent)
	{
		UOpYBOverheadWidget* OverheadWidget = Cast<UOpYBOverheadWidget>(OverheadUIComponent->GetUserWidgetObject());
		if (OverheadWidget)
		{
			OverheadWidget->UpdateHealth(CurrentHealth, MaxHealth);
		}
	}
}

void AOpYBCharacter::OnRep_CurrentAmmo()
{
	// 장전 완료! (탄약 개수가 증가함)
	if (CurrentAmmo > LastAmmoCount)
	{
		// 1발이 채워졌으므로 진행도를 0으로 초기화하여 다음 발 장전을 준비
		LocalReloadProgress = 0.0f;
	}
	// 만약 총을 쏴서 탄약이 줄어든 거라면 진행도는 유지함 (브롤스타즈 방식)

	LastAmmoCount = CurrentAmmo;

	// Tick에서 매 프레임 업데이트하므로 여기선 즉시 업데이트 호출 생략 가능하지만 확실히 하기 위해 한 번 더 호출
	if (OverheadUIComponent)
	{
		UOpYBOverheadWidget* OverheadWidget = Cast<UOpYBOverheadWidget>(OverheadUIComponent->GetUserWidgetObject());
		if (OverheadWidget)
		{
			OverheadWidget->UpdateAmmo(CurrentAmmo, MaxAmmo, LocalReloadProgress);
		}
	}
}

float AOpYBCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HasAuthority())
	{
		CurrentHealth -= ActualDamage;
		if (CurrentHealth <= 0.0f)
		{
			CurrentHealth = 0.0f;
			Die();
		}
		OnRep_CurrentHealth(); // 서버 본인의 UI 갱신을 위해 직접 호출
	}

	return ActualDamage;
}

void AOpYBCharacter::Die()
{
	if (!HasAuthority()) return;
	
	bIsDead = true;
	OnRep_IsDead(); // 서버 호스트용 직접 호출
	
	// 10초 후 자동 리스폰 타이머 시작
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AOpYBCharacter::Respawn, 10.0f, false);
}

void AOpYBCharacter::Respawn()
{
	if (!HasAuthority()) return;
	
	// 랜덤 스폰 위치 찾기
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	if (PlayerStarts.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		AActor* SelectedStart = PlayerStarts[RandomIndex];
		SetActorLocationAndRotation(SelectedStart->GetActorLocation(), SelectedStart->GetActorRotation());
	}
	
	CurrentHealth = MaxHealth;
	CurrentAmmo = MaxAmmo;
	LastAmmoCount = MaxAmmo;
	
	OnRep_CurrentHealth();
	OnRep_CurrentAmmo();

	bIsDead = false;
	OnRep_IsDead(); // 서버 호스트용 직접 호출
}

void AOpYBCharacter::OnRep_IsDead()
{
	if (bIsDead)
	{
		GetMesh()->SetHiddenInGame(true);
		SetActorEnableCollision(false);
		if (OverheadUIComponent) OverheadUIComponent->SetVisibility(false); // 머리 위 체력바 숨김

		if (IsLocallyControlled())
		{
			RespawnTimeLeft = 10.0f;
			if (DeathScreenClass && !DeathScreenInstance)
			{
				DeathScreenInstance = CreateWidget<UOpYBDeathScreenWidget>(GetWorld(), DeathScreenClass);
				if (DeathScreenInstance)
				{
					DeathScreenInstance->AddToViewport();
				}
			}
			DisableInput(Cast<APlayerController>(GetController()));
		}
	}
	else
	{
		GetMesh()->SetHiddenInGame(false);
		SetActorEnableCollision(true);
		if (OverheadUIComponent) OverheadUIComponent->SetVisibility(true);

		if (IsLocallyControlled())
		{
			if (DeathScreenInstance)
			{
				DeathScreenInstance->RemoveFromParent();
				DeathScreenInstance = nullptr;
			}
			EnableInput(Cast<APlayerController>(GetController()));
		}
	}
}
