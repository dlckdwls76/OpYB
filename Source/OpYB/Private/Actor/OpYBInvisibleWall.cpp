#include "Actor/OpYBInvisibleWall.h"
#include "Components/BoxComponent.h"

// Sets default values
AOpYBInvisibleWall::AOpYBInvisibleWall()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;

	// Set default size
	CollisionBox->InitBoxExtent(FVector(100.f, 100.f, 500.f));	

	// Block all movement
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Hide the actor in game
	SetActorHiddenInGame(true);
}

// Called when the game starts or when spawned
void AOpYBInvisibleWall::BeginPlay()
{
	Super::BeginPlay();
}
