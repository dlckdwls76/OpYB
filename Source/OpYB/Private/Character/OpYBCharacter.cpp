// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/OpYBCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Actor/OpYBProjectile.h"
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
	GetCharacterMovement()->bConstrainToPlane = false; // 구르기나 점프 등을 위해 평면 제약 해제
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
	FireRate = 0.1f; // 0.1초마다 1발 (1초에 10발)
	LastFireTime = 0.0f;
}

void AOpYBCharacter::BeginPlay()
{
	Super::BeginPlay();

	// stub
}

void AOpYBCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// stub
}

void AOpYBCharacter::PlayRollMontage()
{
	if (RollMontage)
	{
		PlayAnimMontage(RollMontage);
	}
}

void AOpYBCharacter::AttemptShoot()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	
	// 쿨타임(연사 속도) 체크
	if (CurrentTime - LastFireTime >= FireRate)
	{
		LastFireTime = CurrentTime;

		// 캐릭터 캡슐과 아예 겹치지 않도록 스폰 거리를 150으로 넉넉히 벌립니다.
		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 150.0f + FVector(0, 0, 20.0f);
		FRotator SpawnRotation = GetActorRotation();

		// ServerRPC to spawn the projectile
		ServerShoot(SpawnLocation, SpawnRotation);
	}
}

bool AOpYBCharacter::ServerShoot_Validate(FVector SpawnLocation, FRotator SpawnRotation)
{
	return true;
}

void AOpYBCharacter::ServerShoot_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (ProjectileClass)
	{
		// Deferred Spawning (지연 생성)을 사용하여, 물리 충돌이 계산되기 전에 Owner와 Instigator를 완벽하게 주입합니다.
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		AOpYBProjectile* Proj = GetWorld()->SpawnActorDeferred<AOpYBProjectile>(ProjectileClass, SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		if (Proj)
		{
			// 생성을 완료하며 물리 엔진에 등록
			Proj->FinishSpawning(SpawnTransform);
		}
	}
}
