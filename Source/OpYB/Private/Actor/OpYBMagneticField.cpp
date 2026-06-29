	#include "Actor/OpYBMagneticField.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/OpYBCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AOpYBMagneticField::AOpYBMagneticField()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// Typically a large sphere or cylinder with a translucent material that is set to NoCollision
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called when the game starts or when spawned
void AOpYBMagneticField::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Start damage timer
		GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AOpYBMagneticField::ApplyDamageToPlayersOutside, DamageInterval, true);
	}
}

// Called every frame
void AOpYBMagneticField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		// Shrink logic
		if (CurrentRadius > MinRadius)
		{
			CurrentRadius -= ShrinkRate * DeltaTime;
			if (CurrentRadius < MinRadius)
			{
				CurrentRadius = MinRadius;
			}
		}
	}

	// Update the scale of the mesh visually on both server and client
	// Assuming the default sphere has a diameter of 100 units
	float ScaleFactor = CurrentRadius / 50.0f;
	SetActorScale3D(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
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
				// Character is outside the safe zone
				UGameplayStatics::ApplyDamage(Char, DamagePerTick, nullptr, this, UDamageType::StaticClass());
			}
		}
	}
}
