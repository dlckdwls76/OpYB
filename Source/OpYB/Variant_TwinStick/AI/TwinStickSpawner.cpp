// 에픽게임즈 저작권 소유.


#include "TwinStickSpawner.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Kismet/GameplayStatics.h"
#include "TwinStickNPC.h"
#include "TwinStickGameMode.h"

ATwinStickSpawner::ATwinStickSpawner()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void ATwinStickSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨에서 리캐스트 내비메시 액터 찾기
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARecastNavMesh::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		NavData = Cast<ARecastNavMesh>(ActorList[0]);
	} else {

		UE_LOG(LogTemp, Log, TEXT("Could not find recast navmesh"));

	}

	// 스폰 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(SpawnGroupTimer, this, &ATwinStickSpawner::SpawnNPCGroup, SpawnGroupDelay, true);

	// 첫 번째 NPC 그룹 스폰
	SpawnNPCGroup();
}

void ATwinStickSpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 스폰 타이머 지우기
	GetWorld()->GetTimerManager().ClearTimer(SpawnGroupTimer);
	GetWorld()->GetTimerManager().ClearTimer(SpawnNPCTimer);
}

void ATwinStickSpawner::SpawnNPCGroup()
{
	// 그룹 스폰 카운터 초기화
	SpawnCount = 0;

	// 아직 최대 NPC 제한 미만인지 확인
	if (ATwinStickGameMode* GM = Cast<ATwinStickGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->CanSpawnNPCs())
		{
			SpawnNPC();
		}
	}
}

void ATwinStickSpawner::SpawnNPC()
{
	FTransform SpawnTransform;

	// 스포너 주변의 무작위 지점 찾기
	FVector SpawnLoc;
	if (UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), GetActorLocation(), SpawnLoc, SpawnRadius, NavData))
	{
		SpawnTransform.SetLocation(SpawnLoc);

		// NPC 스폰
		ATwinStickNPC* NPC = GetWorld()->SpawnActor<ATwinStickNPC>(NPCClass, SpawnTransform);
	}

	// 스폰 카운터 증가
	++SpawnCount;

	// 스폰할 적이 아직 남아 있습니까?
	if (SpawnCount < SpawnGroupSize)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnNPCTimer, this, &ATwinStickSpawner::SpawnNPC, FMath::RandRange(MinSpawnDelay, MaxSpawnDelay), false);
	}

}
