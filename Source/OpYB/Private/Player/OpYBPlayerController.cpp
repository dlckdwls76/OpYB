// 에픽게임즈 저작권 소유.

#include "Player/OpYBPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Character/OpYBCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"
#include "UI/OpYBHUD.h"
#include "OpYB.h"
#include "Component/OpYBCombatComponent.h"

AOpYBPlayerController::AOpYBPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 마우스 좌표 추적을 유지하기 위해 true로 두되, 
	// 실제 윈도우 마우스 포인터는 EMouseCursor::None으로 지정해 완벽히 숨깁니다.
	bShowMouseCursor = true; 
	DefaultMouseCursor = EMouseCursor::None;
}

void AOpYBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어(실제 화면을 보고 조작하는 유저)일 때만 입력 설정을 적용합니다.
	if (IsLocalPlayerController())
	{
		// 위젯이 키보드 포커스를 뺏어가는 것을 방지 (사격 전 이동 불가 버그 수정)
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			// DefaultMappingContext를 제거하여 마우스 좌클릭 이동을 막습니다.
			if (DefaultMapingContext)
			{
				Subsystem->AddMappingContext(DefaultMapingContext, 1);
				UE_LOG(LogTemp, Warning, TEXT("Added DefaultMapingContext successfully in BeginPlay."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to find UEnhancedInputLocalPlayerSubsystem in BeginPlay!"));
		}
	}
}

void AOpYBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		
		// 액션 바인딩 설정
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// 무브먼트 입력 설정
			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AOpYBPlayerController::MoveForward);
			EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AOpYBPlayerController::MoveRight);
			
			// 사격 입력 설정 (클릭 -> 뗄 때 발사되도록 Completed로 설정)
			if (ShootAction)
			{
				EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Triggered, this, &AOpYBPlayerController::Shoot);
			}

			// 궁극기 입력 (R키)
			if (UltimateAction)
			{
				EnhancedInputComponent->BindAction(UltimateAction, ETriggerEvent::Started, this, &AOpYBPlayerController::ToggleUltimate);
			}

			// 구르기 입력 설정 (스페이스바)
			if (RollAction)
			{
				EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AOpYBPlayerController::DoRoll);
			}

			UE_LOG(LogTemp, Warning, TEXT("Successfully bound Input Actions."));
		}
	}
}

void AOpYBPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 마우스의 시선을 따라 다니지만 WASD에 관여하지 않기 (회전 처리)
	if (AOpYBCharacter* ControlledPawn = Cast<AOpYBCharacter>(GetPawn()))
	{
		// 구르기 중에는 회전하지 않음
		if (ControlledPawn->IsRolling())
		{
			return;
		}

		FVector WorldLocation, WorldDirection;
		// 화면의 마우스 2D 좌표를 3D 공간의 레이(Ray)로 변환
		if (DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			FVector PawnLocation = ControlledPawn->GetActorLocation();
			float PlaneZ = PawnLocation.Z;

			// 마우스 레이가 캐릭터와 같은 높이(Z)의 가상 평면과 만나는 지점을 계산
			if (FMath::Abs(WorldDirection.Z) > KINDA_SMALL_NUMBER)
			{
				float t = (PlaneZ - WorldLocation.Z) / WorldDirection.Z;
				FVector TargetLocation = WorldLocation + WorldDirection * t;

				FVector Direction = (TargetLocation - PawnLocation).GetSafeNormal();
				FRotator NewRotation = Direction.Rotation();
				NewRotation.Pitch = 0.f; // 캐릭터를 똑바로 유지
				NewRotation.Roll = 0.f;

				ControlledPawn->SetActorRotation(NewRotation);

				// 멀티플레이어 동기화를 위해 서버로 회전값 전송 (로컬 플레이어일 때만)
				if (IsLocalPlayerController() && HasAuthority() == false)
				{
					TimeSinceLastRotationSync += DeltaTime;
					
					// 초당 최대 20회(0.05초 간격)로 제한하고, 회전값이 1도 이상 변했을 때만 전송
					if (TimeSinceLastRotationSync >= 0.05f)
					{
						if (!LastSentRotation.Equals(NewRotation, 1.0f))
						{
							ServerSetPawnRotation(NewRotation);
							LastSentRotation = NewRotation;
						}
						TimeSinceLastRotationSync = 0.0f;
					}
				}
			}
		}
	}
}

void AOpYBPlayerController::MoveForward(const FInputActionValue& Value)
{
	float MovementValue = Value.Get<float>();
	if (AOpYBCharacter* ControlledPawn = Cast<AOpYBCharacter>(GetPawn()))
	{
		if (ControlledPawn->IsRolling()) return; // 구르기 중 이동 불가

		// 12시 방향 (World +X)
		ControlledPawn->AddMovementInput(FVector(1.f, 0.f, 0.f), MovementValue, false);
	}
}

void AOpYBPlayerController::MoveRight(const FInputActionValue& Value)
{
	float MovementValue = Value.Get<float>();
	if (AOpYBCharacter* ControlledPawn = Cast<AOpYBCharacter>(GetPawn()))
	{
		if (ControlledPawn->IsRolling()) return; // 구르기 중 이동 불가

		// 3시 방향 (World +Y)
		ControlledPawn->AddMovementInput(FVector(0.f, 1.f, 0.f), MovementValue, false);
	}
}

void AOpYBPlayerController::Shoot()
{
	UE_LOG(LogTemp, Warning, TEXT("1. [PlayerController] 마우스 클릭 감지됨!"));
	if (AOpYBCharacter* ControlledCharacter = Cast<AOpYBCharacter>(GetPawn()))
	{
		if (ControlledCharacter->GetCombatComponent())
		{
			ControlledCharacter->GetCombatComponent()->AttemptShoot(bIsUltReadyMode);
		}

		// 궁극기 모드에서 쐈다면 즉시 모드 해제 및 에임 복구
		if (bIsUltReadyMode)
		{
			bIsUltReadyMode = false;
			if (AOpYBHUD* HUD = Cast<AOpYBHUD>(GetHUD()))
			{
				HUD->SetAimMode(false);
			}
		}

		// 사격 시 에임 반동 애니메이션 (HUD에 요청)
		if (AOpYBHUD* HUD = Cast<AOpYBHUD>(GetHUD()))
		{
			HUD->PlayShootAnimation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. [PlayerController] 조종 중인 캐릭터를 찾을 수 없습니다."));
	}
}

void AOpYBPlayerController::ToggleUltimate()
{
	if (AOpYBCharacter* ControlledCharacter = Cast<AOpYBCharacter>(GetPawn()))
	{
		// 게이지가 다 찼을 때만 토글 가능
		if (ControlledCharacter->CanUseUltimate())
		{
			bIsUltReadyMode = !bIsUltReadyMode;
			
			if (AOpYBHUD* HUD = Cast<AOpYBHUD>(GetHUD()))
			{
				HUD->SetAimMode(bIsUltReadyMode);
			}
		}
	}
}

void AOpYBPlayerController::OnUltimateAction()
{
	if (AOpYBCharacter* ControlledCharacter = Cast<AOpYBCharacter>(GetPawn()))
	{
		ControlledCharacter->ToggleUltimateAim();
	}
}

bool AOpYBPlayerController::ServerSetPawnRotation_Validate(FRotator NewRotation)
{
	return true;
}

void AOpYBPlayerController::ServerSetPawnRotation_Implementation(FRotator NewRotation)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->SetActorRotation(NewRotation);
	}
}

void AOpYBPlayerController::DoRoll()
{
	if (AOpYBCharacter* ControlledCharacter = Cast<AOpYBCharacter>(GetPawn()))
	{
		ControlledCharacter->PlayRollMontage();
	}
}

