// 프로젝트 세팅의 설명 페이지에 저작권 공지를 채우세요.

#include "Actor/OpYBProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpYBCharacter.h"

// 기본값을 설정합니다.
AOpYBProjectile::AOpYBProjectile()
{
	// 이 액터가 매 프레임 Tick()을 호출하도록 설정합니다. 필요하지 않은 경우 성능 향상을 위해 끌 수 있습니다.
	PrimaryActorTick.bCanEverTick = true;

	// 단순한 충돌 표현을 위해 구체를 사용합니다.
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(15.0f);
	// 플레이어 밀림을 방지하기 위해 커스텀 충돌 사용
	CollisionComp->BodyInstance.SetCollisionProfileName("Custom");
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block); // 기본적으로 벽 등은 다 막힘(Block)
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// ★핵심: 캐릭터(Pawn)와는 겹치도록(Overlap) 설정하여 물리적 폭발 방지
	CollisionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore); // 마우스 커서 레이캐스트 무시
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// 벽에 부딪혔을 때 (Block)
	CollisionComp->OnComponentHit.AddDynamic(this, &AOpYBProjectile::OnHit);
	// 캐릭터에 겹쳤을 때 (Overlap)
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AOpYBProjectile::OnOverlapBegin);
	// 플레이어가 밟고 걸을 수 없습니다.
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	// 루트 컴포넌트로 설정
	RootComponent = CollisionComp;
	// 메시 컴포넌트
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision")); // 메시는 충돌이 필요 없고, 구체는 충돌이 필요합니다.

	// 기본 구체 메시 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMeshAsset.Object);
		// 블루프린트에서 크기를 자유롭게 조절할 수 있도록 C++의 강제 스케일 고정 코드를 삭제했습니다.
		// MeshComp->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	}

	// 이 투사체의 움직임을 제어하기 위해 ProjectileMovementComponent를 사용합니다.
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f; // 중력 없음

	// 기본적으로 3초 후 소멸 (속도 * 수명 = 거리이므로 "거리 이동 후 소멸"을 암시적으로 처리함)
	InitialLifeSpan = 1.0f; // 3000 * 1 = 3000 units range

	// 멀티플레이어를 위한 리플리케이션 보장
	bReplicates = true;
}

// 게임이 시작되거나 스폰될 때 호출됩니다.
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

// 매 프레임 호출됩니다.
void AOpYBProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOpYBProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;

	// 벽이나 다른 오브젝트에 부딪혀서 파괴
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit wall/object: %s, Destroying."), *OtherActor->GetName());
		Destroy();
	}
}

void AOpYBProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                          const FHitResult& SweepResult)
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
		UE_LOG(LogTemp, Warning, TEXT("Projectile hit Enemy: %s, Dealing %f Damage."), *OtherActor->GetName(), Damage);

		// 적에게 데미지 입히기 로직 실행 (서버 권한)
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

		// 궁극기 게이지 충전
		if (AOpYBCharacter* InstigatorPawn = Cast<AOpYBCharacter>(GetInstigator()))
		{
			InstigatorPawn->AddUltimateHit();
		}

		Destroy();
	}
}
