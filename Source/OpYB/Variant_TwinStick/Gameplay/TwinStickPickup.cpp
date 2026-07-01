// 에픽게임즈 저작권 소유.


#include "TwinStickPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "TwinStickCharacter.h"
#include "Components/StaticMeshComponent.h"

ATwinStickPickup::ATwinStickPickup()
{
 	PrimaryActorTick.bCanEverTick = true;

	// 루트 생성 component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 충돌 구체 생성
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	CollisionSphere->SetupAttachment(RootComponent);

	CollisionSphere->SetSphereRadius(100.0f);
	CollisionSphere->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 메시 생성
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CollisionSphere);

	Mesh->SetCollisionProfileName(FName("NoCollision"));

}

void ATwinStickPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// 플레이어 캐릭터와 겹쳤습니까?
	if (ATwinStickCharacter* PlayerCharacter = Cast<ATwinStickCharacter>(OtherActor))
	{
		// 플레이어에게 픽업 주기
		PlayerCharacter->AddPickup();

		// 이 픽업 파괴
		Destroy();
	}
}