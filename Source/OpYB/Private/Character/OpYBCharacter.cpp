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
	FireRate = 0.1f; // 0.1초마다 1발 (단발 사격 시 답답함 방지)
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

	UE_LOG(LogTemp, Warning, TEXT("2. [Character] AttemptShoot 호출됨! (쿨타임 검사 중...)"));

	// 쿨타임(연사 속도) 체크 (이게 없으면 1초에 60발이 나가서 총알끼리 부딪혀서 터집니다!)
	if (CurrentTime - LastFireTime >= FireRate)
	{
		LastFireTime = CurrentTime;

		UE_LOG(LogTemp, Warning, TEXT("3. [Character] 쿨타임 통과! (발사 승인)"));

		// 총알이 캐릭터 몸 정중앙(내부)에서부터 발사되도록 앞쪽 거리(Offset)를 없앱니다.
		// Z축으로만 살짝(20) 올려서 총구 높이 정도에 맞춥니다.
		FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 20.0f);
		FRotator SpawnRotation = GetActorRotation();
		
		// ServerRPC to spawn the projectile
		ServerShoot(SpawnLocation, SpawnRotation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. [Character] 쿨타임에 막힘! (아직 안 나감)"));
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
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("X. [Server] ProjectileClass가 블루프린트에 등록되어 있지 않습니다!!"));
	}
}
