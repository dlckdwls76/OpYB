// 에픽게임즈 저작권 소유.


#include "TwinStickProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TwinStickNPC.h"

ATwinStickProjectile::ATwinStickProjectile()
{
 	PrimaryActorTick.bCanEverTick = true;

	// 이 액터는 InitialLifeSpan이 만료되면 자동으로 파괴됩니다.
	InitialLifeSpan = 2.0f;

	// 충돌 구체 생성 and set it as the root component
	RootComponent = CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

	CollisionSphere->SetSphereRadius(35.0f);
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);

	// 메시 생성
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	Mesh->SetCollisionProfileName(FName("NoCollision"));

	// 투사체 이동 컴포넌트 생성. 씬 컴포넌트가 아니므로 붙일 필요가 없습니다.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 15000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bRotationRemainsVertical = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->bForceSubStepping = true;

	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ATwinStickProjectile::OnProjectileStop);
}

void ATwinStickProjectile::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// NPC를 맞췄습니까?
	if (ATwinStickNPC* NPC = Cast<ATwinStickNPC>(Other))
	{
		// NPC에게 맞았음을 알림
		NPC->ProjectileImpact(FVector::ZeroVector);

		// 이 투사체 파괴
		Destroy();
	}
}

void ATwinStickProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	// 이 액터 파괴 immediately
	Destroy();
}
