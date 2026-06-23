// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/OpYBProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

// Sets default values
AOpYBProjectile::AOpYBProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	// Use custom collision to prevent physical pushes on players
	CollisionComp->BodyInstance.SetCollisionProfileName("Custom");
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block); // 기본적으로 벽 등은 다 막힘(Block)
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // ★핵심: 캐릭터(Pawn)와는 겹치도록(Overlap) 설정하여 물리적 폭발 방지
	CollisionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // 마우스 커서 레이캐스트 무시
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// 벽에 부딪혔을 때 (Block)
	CollisionComp->OnComponentHit.AddDynamic(this, &AOpYBProjectile::OnHit);
	// 캐릭터에 겹쳤을 때 (Overlap)
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AOpYBProjectile::OnOverlapBegin);

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Mesh component
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision")); // Mesh doesn't need collision, Sphere does
	
	// Load default sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMeshAsset.Object);
		MeshComp->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f)); // Scale it down
	}

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // No gravity

	// Die after 3 seconds by default (this handles the "disappear after distance" implicitly since speed * life = distance)
	InitialLifeSpan = 1.0f; // 3000 * 1 = 3000 units range

	// Ensure replication for multiplayer
	bReplicates = true;
}

// Called when the game starts or when spawned
void AOpYBProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("Projectile BeginPlay! Spawning bullet at %s"), *GetActorLocation().ToString());

	// 자신을 발사한 주체(플레이어)와는 물리적으로 충돌하지 않도록 설정
	if (GetOwner())
	{
		CollisionComp->IgnoreActorWhenMoving(GetOwner(), true);
	}
	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

// Called every frame
void AOpYBProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOpYBProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;
	
	// 벽이나 다른 오브젝트에 부딪혀서 파괴
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit wall/object: %s, Destroying."), *OtherActor->GetName());
		Destroy();
	}
}

void AOpYBProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this) return;

	// 발사한 주체(플레이어 본인)와 겹쳤다면 완전히 무시 (파괴 안됨)
	if (OtherActor == GetOwner() || OtherActor == GetInstigator()) 
	{
		return;
	}

	// 다른 총알끼리 겹치면 무시
	if (OtherActor->IsA(AOpYBProjectile::StaticClass())) 
	{
		return;
	}

	// 그 외의 캐릭터(적 등)에 겹쳤다면
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit Enemy: %s, Destroying."), *OtherActor->GetName());
		
		// TODO: 적에게 데미지 입히기 로직 추가

		Destroy();
	}
}
