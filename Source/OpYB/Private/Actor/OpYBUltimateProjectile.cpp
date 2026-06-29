#include "Actor/OpYBUltimateProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AOpYBUltimateProjectile::AOpYBUltimateProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Use a sphere as a simple collision representation. Make it large.
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(150.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("OverlapAllDynamic");
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AOpYBUltimateProjectile::OnOverlapBegin);

	// Set as root component
	RootComponent = CollisionComp;

	// Use a StaticMeshComponent to represent the projectile
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision")); // Visual only
	MeshComp->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// Die after 5 seconds by default
	InitialLifeSpan = 5.0f;
}

void AOpYBUltimateProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AOpYBUltimateProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOpYBUltimateProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only add impulse and destroy projectile if we hit a character
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr))
	{
		// 자기 자신(발사자)은 무시
		if (OtherActor == GetInstigator())
		{
			return;
		}

		ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);
		if (HitCharacter)
		{
			// 데미지 적용
			UGameplayStatics::ApplyDamage(HitCharacter, DamageAmount, GetInstigatorController(), this, UDamageType::StaticClass());

			// 넉백 적용 (발사 방향으로 넉백)
			FVector LaunchDirection = GetActorForwardVector();
			LaunchDirection.Z = 0.5f; // 약간 위로 띄움
			LaunchDirection.Normalize();

			FVector LaunchVelocity = LaunchDirection * KnockbackStrength;
			
			// xy override, z override 모두 true로 주어 강력하게 밀어냄
			HitCharacter->LaunchCharacter(LaunchVelocity, true, true);
		}
	}
}
