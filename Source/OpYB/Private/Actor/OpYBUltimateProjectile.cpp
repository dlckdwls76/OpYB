#include "Actor/OpYBUltimateProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"

AOpYBUltimateProjectile::AOpYBUltimateProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	// 충돌 구체가 너무 크면 스폰되자마자 바닥(Landscape)에 파묻혀서 충돌 인식이 씹히고 땅으로 꺼지게 됩니다. 
	// 그래서 시각적 크기(Mesh)는 유지하되 물리 충돌체 크기만 줄입니다.
	CollisionComp->InitSphereRadius(30.0f);
	CollisionComp->SetCollisionProfileName("BlockAllDynamic"); // 랜드스케이프를 포함한 모든 것과 무조건 부딪히게 설정
	CollisionComp->OnComponentHit.AddDynamic(this, &AOpYBUltimateProjectile::OnHit);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AOpYBUltimateProjectile::OnOverlapBegin);

	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision")); // Visual only
	MeshComp->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));

	DangerZoneDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("DangerZoneDecal"));
	DangerZoneDecal->SetupAttachment(RootComponent);
	// 랜드스케이프 같은 울퉁불퉁한 지형에서도 잘 보이도록 투영 깊이(X값)를 200 정도로 넉넉하게 줍니다.
	DangerZoneDecal->DecalSize = FVector(200.0f, ExplosionRadius, ExplosionRadius);
	DangerZoneDecal->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	InitialLifeSpan = 10.0f;
}

void AOpYBUltimateProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 발사자(자신)와 쏘자마자 부딪혀서 제자리에서 터지는 것 방지
	if (GetInstigator())
	{
		CollisionComp->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	// Danger zone decal – replicate to all clients at target location
	if (DangerZoneDecal)
	{
		DangerZoneDecal->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DangerZoneDecal->SetWorldLocation(TargetLocation);
	}

	// 위에서 아래로 떨어지는 묵직한 느낌(박격포 같은 곡사)을 주기 위해 CustomArc 방식 사용
	FVector OutVelocity = FVector::ZeroVector;
	
	// ArcValue: 0.0(직선) ~ 1.0(수직). 0.7 정도면 꽤 높이 솟구쳤다 떨어지는 느낌을 줍니다.
	bool bFoundTossVelocity = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		this, OutVelocity, GetActorLocation(), TargetLocation, GetWorld()->GetGravityZ(), 0.75f);

	if (bFoundTossVelocity)
	{
		ProjectileMovement->Velocity = OutVelocity;
	}
	else
	{
		ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	}
}

void AOpYBUltimateProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOpYBUltimateProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority()) return;

	// 자신이나 발사자가 아닐 경우 무조건 폭발 (OtherActor가 랜드스케이프처럼 null로 잡히는 특수 경우도 포함)
	if (OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	MulticastExplode();
	MulticastDestroy();
}

void AOpYBUltimateProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	if (OtherActor && OtherActor != this && OtherActor != GetInstigator())
	{
		// 적이든 바닥이든 아무거나 부딪히면 터지도록 설계
		MulticastExplode();
		MulticastDestroy();
	}
}

void AOpYBUltimateProjectile::MulticastExplode_Implementation()
{
	if (HasAuthority())
	{
		// 반경 내 모든 액터에게 데미지 적용
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);
		IgnoredActors.Add(GetInstigator());

		UGameplayStatics::ApplyRadialDamageWithFalloff(
			this, DamageAmount, DamageAmount / 2.0f, GetActorLocation(), 
			ExplosionRadius / 2.0f, ExplosionRadius, 1.0f, UDamageType::StaticClass(), 
			IgnoredActors, this, GetInstigatorController());

		// 폭발 반경 내 캐릭터 찾아서 넉백 적용
		TArray<FHitResult> HitResults;
		FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
		bool bHit = GetWorld()->SweepMultiByChannel(HitResults, GetActorLocation(), GetActorLocation(), FQuat::Identity, ECC_Pawn, SphereShape);
		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				ACharacter* HitChar = Cast<ACharacter>(Hit.GetActor());
				if (HitChar && HitChar != GetInstigator())
				{
					FVector Dir = HitChar->GetActorLocation() - GetActorLocation();
					Dir.Z = 0.5f; // 약간 위로 띄움
					Dir.Normalize();
					HitChar->LaunchCharacter(Dir * KnockbackStrength, true, true);
				}
			}
		}
	}

	// 파티클은 모든 클라이언트에서 스폰되도록 처리
	if (ExplosionEffect)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation(), FRotator::ZeroRotator, FVector(3.f));
		if (PSC)
		{
			PSC->bAutoDestroy = true; // 재생이 자연스럽게 끝나면 자동 소멸
			
			// 무한 루프 파티클일 경우를 대비해 5초 뒤 강제 소멸 타이머 설정
			FTimerHandle DestroyTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				DestroyTimerHandle,
				FTimerDelegate::CreateWeakLambda(PSC, [PSC]()
				{
					if (IsValid(PSC))
					{
						PSC->DestroyComponent();
					}
				}),
				5.0f,
				false
			);
		}
	}
}

void AOpYBUltimateProjectile::MulticastDestroy_Implementation()
{
	// 데칼 제거
	if (DangerZoneDecal)
	{
		DangerZoneDecal->DestroyComponent();
	}
	// 투사체 삭제
	Destroy();
}

void AOpYBUltimateProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOpYBUltimateProjectile, TargetLocation);
	DOREPLIFETIME(AOpYBUltimateProjectile, DangerZoneDecal);
}
