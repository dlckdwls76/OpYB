// 에픽게임즈 저작권 소유.


#include "TwinStickAoEAttack.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "TwinStickNPC.h"

ATwinStickAoEAttack::ATwinStickAoEAttack()
{
 	PrimaryActorTick.bCanEverTick = true;

	// 루트 생성 component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 메시 생성 that provides the visual representation for the AoE
	SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere Visual"));
	SphereVisual->SetupAttachment(RootComponent);

	SphereVisual->SetCollisionProfileName(FName("NoCollision"));

	// 충돌 구체 생성
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	CollisionSphere->SetupAttachment(RootComponent);

	CollisionSphere->SetSphereRadius(750.0f);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ATwinStickAoEAttack::OnAoEOverlap);
}

void ATwinStickAoEAttack::BeginPlay()
{
	Super::BeginPlay();
	
	// 광역 공격 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(StartAoETimer, this, &ATwinStickAoEAttack::StartAoE, StartAoETime, false);
	GetWorld()->GetTimerManager().SetTimer(StopAoETimer, this, &ATwinStickAoEAttack::StopAoE, StopAoETime, false);

}

void ATwinStickAoEAttack::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 타이머 지우기
	GetWorld()->GetTimerManager().ClearTimer(StartAoETimer);
	GetWorld()->GetTimerManager().ClearTimer(StopAoETimer);
}

void ATwinStickAoEAttack::StartAoE()
{
	// 활성 플래그 올리기
	bIsAoEActive = true;

	// NPC와 겹치는 모든 액터 찾기
	TArray<AActor*> Overlaps;
	CollisionSphere->GetOverlappingActors(Overlaps, ATwinStickNPC::StaticClass());

	// 겹치는 각 액터 처리
	for (AActor* Current : Overlaps)
	{
		if (ATwinStickNPC* NPC = Cast<ATwinStickNPC>(Current))
		{
			// NPC에게 맞았음을 알림
			NPC->ProjectileImpact(FVector::ZeroVector);
		}
	}
}

void ATwinStickAoEAttack::StopAoE()
{
	// 활성화 플래그 해제
	bIsAoEActive = false;

	// 데미지 틱 타이머 정지
	GetWorld()->GetTimerManager().ClearTimer(StartAoETimer);

	// BP 핸들러 호출. 작업이 끝나면 액터를 파괴하는 역할을 합니다.
	BP_AoEFinished();
}

void ATwinStickAoEAttack::OnAoEOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 폭발이 활성화되었습니까?
	if (bIsAoEActive)
	{
		// NPC와 겹쳤습니까?
		if (ATwinStickNPC* NPC = Cast<ATwinStickNPC>(OtherActor))
		{
			// NPC에게 맞았음을 알림
			NPC->ProjectileImpact(FVector::ZeroVector);
		}
	}
}
