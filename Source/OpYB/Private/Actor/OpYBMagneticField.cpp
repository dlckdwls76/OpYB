#include "Actor/OpYBMagneticField.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpYBCharacter.h"
#include "Net/UnrealNetwork.h"

// 기본값을 설정합니다.
AOpYBMagneticField::AOpYBMagneticField()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 보통 충돌이 없는 반투명 머티리얼이 적용된 큰 구체나 원기둥입니다.
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));

	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComp"));
	PostProcessComp->SetupAttachment(RootComponent);
	PostProcessComp->bUnbound = true;
	PostProcessComp->BlendWeight = 0.0f;

	// 구역 외부 시각 효과 설정 (블러/색조)
	PostProcessComp->Settings.bOverride_VignetteIntensity = true;
	PostProcessComp->Settings.VignetteIntensity = 1.0f;
	PostProcessComp->Settings.bOverride_SceneFringeIntensity = true;
	PostProcessComp->Settings.SceneFringeIntensity = 2.5f; // 색수차
	PostProcessComp->Settings.bOverride_ColorGain = true;
	PostProcessComp->Settings.ColorGain = FVector4(1.5f, 0.6f, 0.6f, 1.0f); // 옅은 붉은색 색조
}

// 게임이 시작되거나 스폰될 때 호출됩니다.
void AOpYBMagneticField::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// 피해 타이머 시작
		GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AOpYBMagneticField::ApplyDamageToPlayersOutside, DamageInterval, true);
	}
}

// 매 프레임 호출됩니다.
void AOpYBMagneticField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// 축소 로직
		if (CurrentRadius > MinRadius)
		{
			CurrentRadius -= ShrinkRate * DeltaTime;
			if (CurrentRadius < MinRadius)
			{
				CurrentRadius = MinRadius;
			}
		}
	}

	// 서버와 클라이언트 양쪽에서 메시 스케일을 시각적으로 업데이트합니다.
	// 기본 구체의 지름이 100 유닛이라고 가정합니다.
	float ScaleFactor = CurrentRadius / 50.0f;
	SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));

	// 클라이언트 측 시각 효과 업데이트
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (AOpYBCharacter* LocalChar = Cast<AOpYBCharacter>(PC->GetPawn()))
		{
			if (LocalChar->IsLocallyControlled())
			{
				float Distance = FVector::Dist2D(GetActorLocation(), LocalChar->GetActorLocation());
				if (Distance > CurrentRadius)
				{
					PostProcessComp->BlendWeight = FMath::FInterpTo(PostProcessComp->BlendWeight, 1.0f, DeltaTime, 5.0f);
				}
				else
				{
					PostProcessComp->BlendWeight = FMath::FInterpTo(PostProcessComp->BlendWeight, 0.0f, DeltaTime, 5.0f);
				}
			}
		}
	}
}

void AOpYBMagneticField::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOpYBMagneticField, CurrentRadius);
}

void AOpYBMagneticField::ApplyDamageToPlayersOutside()
{
	if (!HasAuthority()) return;

	FVector CenterLocation = GetActorLocation();

	TArray<AActor*> AllCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOpYBCharacter::StaticClass(), AllCharacters);

	for (AActor* Actor : AllCharacters)
	{
		AOpYBCharacter* Char = Cast<AOpYBCharacter>(Actor);
		if (Char && !Char->bIsDead)
		{
			float Distance = FVector::Dist2D(CenterLocation, Char->GetActorLocation());
			if (Distance > CurrentRadius)
			{
				// 캐릭터가 안전 구역 밖에 있습니다.
				UGameplayStatics::ApplyDamage(Char, DamagePerTick, nullptr, this, UDamageType::StaticClass());
			}
		}
	}
}
