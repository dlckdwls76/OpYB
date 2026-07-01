// 에픽게임즈 저작권 소유.


#include "TwinStickNPC.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TwinStickCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TwinStickGameMode.h"
#include "TwinStickPickup.h"
#include "Engine/World.h"
#include "TwinStickNPCDestruction.h"
#include "TimerManager.h"

ATwinStickNPC::ATwinStickNPC()
{
	PrimaryActorTick.bCanEverTick = true;

	// 스폰될 때 AI 컨트롤러를 스폰하는지 확인
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 상속된 컴포넌트 설정
	GetCapsuleComponent()->SetCapsuleRadius(45.0f);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);

	GetMesh()->SetCollisionProfileName(FName("NoCollision"));

	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->BrakingFriction = 1.0f;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 100.0f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseRVOAvoidance = true;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 250.0f;
	GetCharacterMovement()->AvoidanceWeight = 1.0f;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
}

void ATwinStickNPC::BeginPlay()
{
	Super::BeginPlay();

	// 필요한 경우 스폰을 제한하기 위해 NPC 카운터 증가
	if (ATwinStickGameMode* GM = Cast<ATwinStickGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncreaseNPCs();
	}

}

void ATwinStickNPC::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 파괴 타이머 지우기
	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
}

void ATwinStickNPC::Destroyed()
{
	// NPC 카운터 감소 so we can cap spawning if necessary
	if (ATwinStickGameMode* GM = Cast<ATwinStickGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->DecreaseNPCs();
	}

	Super::Destroyed();
}

void ATwinStickNPC::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// 플레이어와 충돌했습니까?
	if (ATwinStickCharacter* PlayerCharacter = Cast<ATwinStickCharacter>(Other))
	{
		// 캐릭터에게 피해 적용
		PlayerCharacter->HandleDamage(1.0f, GetActorForwardVector());
	}
}

void ATwinStickNPC::ProjectileImpact(const FVector& ForwardVector)
{
	// 아직 맞지 않은 경우에만 데미지 처리
	if (bHit)
	{
		return;
	}

	// 명중 플래그 올리기
	bHit = true;

	// 캐릭터 무브먼트 비활성화
	GetCharacterMovement()->Deactivate();

	// 점수 보상
	if (ATwinStickGameMode* GM = Cast<ATwinStickGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->ScoreUpdate(Score);
	}

	// 픽업을 무작위로 스폰
	if (FMath::RandRange(0, 100) < PickupSpawnChance)
	{
		ATwinStickPickup* Pickup = GetWorld()->SpawnActor<ATwinStickPickup>(PickupClass, GetActorTransform());
	}
	
	// NPC 스폰 destruction proxy
	ATwinStickNPCDestruction* DestructionProxy = GetWorld()->SpawnActor<ATwinStickNPCDestruction>(DestructionProxyClass, GetActorTransform());

	// 이 액터 숨기기
	SetActorHiddenInGame(true);

	// 충돌 비활성화
	SetActorEnableCollision(false);

	// 파괴 지연
	GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &ATwinStickNPC::DeferredDestroy, DeferredDestructionTime, false);
}

void ATwinStickNPC::DeferredDestroy()
{
	// 이 액터 파괴
	Destroy();
}
