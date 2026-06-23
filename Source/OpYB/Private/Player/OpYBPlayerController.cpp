// Copyright Epic Games, Inc. All Rights Reserved.

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
#include "OpYB.h"

AOpYBPlayerController::AOpYBPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AOpYBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어(실제 화면을 보고 조작하는 유저)일 때만 입력 설정을 적용합니다.
	if (IsLocalPlayerController())
	{
		// 입력 포커스 설정: 클릭 없이 바로 키보드 입력을 받기 위함
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			// DefaultMappingContext를 제거하여 마우스 좌클릭 이동을 막습니다.
			if (DynamicMappingContext)
			{
				Subsystem->AddMappingContext(DynamicMappingContext, 1);
				UE_LOG(LogTemp, Warning, TEXT("Added DynamicMappingContext successfully in BeginPlay."));
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
		// Create MoveForwardAction if it hasn't been assigned
		if (!MoveForwardAction)
		{
			MoveForwardAction = NewObject<UInputAction>(this, TEXT("MoveForwardAction"));
			MoveForwardAction->ValueType = EInputActionValueType::Axis1D;
		}

		// Create MoveRightAction if it hasn't been assigned
		if (!MoveRightAction)
		{
			MoveRightAction = NewObject<UInputAction>(this, TEXT("MoveRightAction"));
			MoveRightAction->ValueType = EInputActionValueType::Axis1D;
		}

		// Create ShootAction if it hasn't been assigned
		if (!ShootAction)
		{
			ShootAction = NewObject<UInputAction>(this, TEXT("ShootAction"));
			ShootAction->ValueType = EInputActionValueType::Boolean;
		}

		// Create a dynamic mapping context
		DynamicMappingContext = NewObject<UInputMappingContext>(this, TEXT("DynamicMappingContext"));

		// W: MoveForward = 1.0
		DynamicMappingContext->MapKey(MoveForwardAction, EKeys::W);

		// S: MoveForward = -1.0
		FEnhancedActionKeyMapping& MappingS = DynamicMappingContext->MapKey(MoveForwardAction, EKeys::S);
		UInputModifierNegate* NegateS = NewObject<UInputModifierNegate>(DynamicMappingContext);
		MappingS.Modifiers.Add(NegateS);

		// D: MoveRight = 1.0
		DynamicMappingContext->MapKey(MoveRightAction, EKeys::D);

		// A: MoveRight = -1.0
		FEnhancedActionKeyMapping& MappingA = DynamicMappingContext->MapKey(MoveRightAction, EKeys::A);
		UInputModifierNegate* NegateA = NewObject<UInputModifierNegate>(DynamicMappingContext);
		MappingA.Modifiers.Add(NegateA);

		// LeftMouseButton: Shoot
		DynamicMappingContext->MapKey(ShootAction, EKeys::LeftMouseButton);

		// Space: Roll
		// DynamicMappingContext->MapKey(RollAction, EKeys::SpaceBar);

		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Setup movement input
			EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AOpYBPlayerController::MoveForward);
			EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AOpYBPlayerController::MoveRight);
			
			// Setup shoot input (클릭할 때마다 1발씩 발사되도록 Started로 복구)
			EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AOpYBPlayerController::Shoot);
			
			// Setup roll input
			// EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AOpYBPlayerController::DoRoll);
			
			UE_LOG(LogTemp, Warning, TEXT("Successfully bound Input Actions."));
		}
	}
}

void AOpYBPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 마우스의 시선을 따라 다니지만 WASD에 관여하지 않기 (회전 처리)
	if (APawn* ControlledPawn = GetPawn())
	{
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
				NewRotation.Pitch = 0.f; // Keep character upright
				NewRotation.Roll = 0.f;

				ControlledPawn->SetActorRotation(NewRotation);

				// 멀티플레이어 동기화를 위해 서버로 회전값 전송 (로컬 플레이어일 때만)
				if (IsLocalPlayerController() && HasAuthority() == false)
				{
					ServerSetPawnRotation(NewRotation);
				}
			}
		}
	}
}

void AOpYBPlayerController::MoveForward(const FInputActionValue& Value)
{
	float MovementValue = Value.Get<float>();
	if (APawn* ControlledPawn = GetPawn())
	{
		// 12시 방향 (World +X)
		ControlledPawn->AddMovementInput(FVector(1.f, 0.f, 0.f), MovementValue, false);
	}
}

void AOpYBPlayerController::MoveRight(const FInputActionValue& Value)
{
	float MovementValue = Value.Get<float>();
	if (APawn* ControlledPawn = GetPawn())
	{
		// 3시 방향 (World +Y)
		ControlledPawn->AddMovementInput(FVector(0.f, 1.f, 0.f), MovementValue, false);
	}
}

void AOpYBPlayerController::Shoot()
{
	UE_LOG(LogTemp, Warning, TEXT("Shoot Action Triggered!"));
	if (AOpYBCharacter* ControlledCharacter = Cast<AOpYBCharacter>(GetPawn()))
	{
		ControlledCharacter->AttemptShoot();
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

// void AOpYBPlayerController::DoRoll()
// {
// 	if (AOpYBCharacter* ControlledCharacter = Cast<AOpYBCharacter>(GetPawn()))
// 	{
// 		ControlledCharacter->PlayRollMontage();
// 	}
// }
