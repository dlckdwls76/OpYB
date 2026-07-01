#include "Actor/OpYBInvisibleWall.h"
#include "Components/BoxComponent.h"

// 기본값을 설정합니다.
AOpYBInvisibleWall::AOpYBInvisibleWall()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// 기본 크기 설정
	CollisionBox->InitBoxExtent(FVector(100.f, 100.f, 500.f));	

	// 모든 이동 차단
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 게임 내에서 액터 숨기기
	SetActorHiddenInGame(true);
}

// 게임이 시작되거나 스폰될 때 호출됩니다.
void AOpYBInvisibleWall::BeginPlay()
{
	Super::BeginPlay();
}
