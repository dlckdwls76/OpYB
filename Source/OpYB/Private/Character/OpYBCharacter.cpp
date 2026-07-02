// 에픽게임즈 저작권 소유.

#include "Character/OpYBCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/OpYBHUD.h"
#include "UI/OpYBOverheadWidget.h"
#include "UI/OpYBUltimateWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Actor/OpYBProjectile.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "Game/OpYBGameState.h"
#include "Actor/OpYBUltimateProjectile.h"
#include "Component/OpYBHealthComponent.h"
#include "Component/OpYBCombatComponent.h"

AOpYBCharacter::AOpYBCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; 
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true; 
	GetCharacterMovement()->bSnapToPlaneAtStart = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	OverheadUIComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadUI"));
	OverheadUIComponent->SetupAttachment(RootComponent);
	OverheadUIComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	OverheadUIComponent->SetWidgetSpace(EWidgetSpace::Screen); 
	OverheadUIComponent->SetDrawSize(FVector2D(150.0f, 60.0f));
	OverheadUIComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); 

	HealthComponent = CreateDefaultSubobject<UOpYBHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UOpYBCombatComponent>(TEXT("CombatComponent"));

	MaxUltCharge = 3;
	CurrentUltCharge = 0;
}

void AOpYBCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AOpYBCharacter::UpdateHealthUI);
		HealthComponent->OnDeath.AddDynamic(this, &AOpYBCharacter::HandleDeath);
		HealthComponent->OnRespawn.AddDynamic(this, &AOpYBCharacter::HandleRespawn);
		HealthComponent->OnRespawnTimeUpdated.AddDynamic(this, &AOpYBCharacter::UpdateDeathScreenTime);
		
		UpdateHealthUI(HealthComponent->CurrentHealth, HealthComponent->MaxHealth);
	}

	if (CombatComponent)
	{
		CombatComponent->OnAmmoChanged.AddDynamic(this, &AOpYBCharacter::UpdateAmmoUI);
		UpdateAmmoUI(CombatComponent->CurrentAmmo, CombatComponent->MaxAmmo, CombatComponent->LocalReloadProgress);
	}

	bIsDead = false;
	
	if (HasAuthority()) CurrentUltCharge = 0;

	OnRep_CurrentUltCharge();

	if (HasAuthority())
	{
		if (AOpYBGameState* GS = GetWorld()->GetGameState<AOpYBGameState>())
		{
			GS->AddAlivePlayer();
		}
	}

	if (IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimer(CameraOcclusionTimerHandle, this, &AOpYBCharacter::CheckCameraOcclusion, 0.1f, true);
	}
}

void AOpYBCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AOpYBCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOpYBCharacter, CurrentUltCharge);
	DOREPLIFETIME(AOpYBCharacter, UltimateHitCount);
	DOREPLIFETIME(AOpYBCharacter, bIsUltimateReady);
}

void AOpYBCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AOpYBCharacter::PlayRollMontage()
{
	if (RollMontage && !IsRolling())
	{
		PlayAnimMontage(RollMontage);
	}
}

bool AOpYBCharacter::IsRolling() const
{
	if (GetMesh() && GetMesh()->GetAnimInstance() && RollMontage)
	{
		return GetMesh()->GetAnimInstance()->Montage_IsPlaying(RollMontage);
	}
	return false;
}

void AOpYBCharacter::UpdateHealthUI(float InCurrentHealth, float InMaxHealth)
{
	if (OverheadUIComponent)
	{
		UOpYBOverheadWidget* OverheadWidget = Cast<UOpYBOverheadWidget>(OverheadUIComponent->GetUserWidgetObject());
		if (OverheadWidget)
		{
			OverheadWidget->UpdateHealth(InCurrentHealth, InMaxHealth);
		}
	}
}

void AOpYBCharacter::UpdateAmmoUI(int32 InCurrentAmmo, int32 InMaxAmmo, float InReloadProgress)
{
	if (OverheadUIComponent)
	{
		UOpYBOverheadWidget* OverheadWidget = Cast<UOpYBOverheadWidget>(OverheadUIComponent->GetUserWidgetObject());
		if (OverheadWidget)
		{
			OverheadWidget->UpdateAmmo(InCurrentAmmo, InMaxAmmo, InReloadProgress);
		}
	}
}

float AOpYBCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (HealthComponent)
	{
		ActualDamage = HealthComponent->HandleTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	return ActualDamage;
}

void AOpYBCharacter::HandleDeath()
{
	bIsDead = true;
	GetMesh()->SetHiddenInGame(true);
	SetActorEnableCollision(false);
	if (OverheadUIComponent) OverheadUIComponent->SetVisibility(false);

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AOpYBHUD* HUD = Cast<AOpYBHUD>(PC->GetHUD()))
			{
				HUD->ShowDeathScreen();
			}
			DisableInput(PC);
		}
	}
}

void AOpYBCharacter::HandleRespawn()
{
	bIsDead = false;
	GetMesh()->SetHiddenInGame(false);
	SetActorEnableCollision(true);
	if (OverheadUIComponent) OverheadUIComponent->SetVisibility(true);

	if (CombatComponent && HasAuthority())
	{
		CombatComponent->ResetAmmo();
	}

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AOpYBHUD* HUD = Cast<AOpYBHUD>(PC->GetHUD()))
			{
				HUD->HideDeathScreen();
			}
			EnableInput(PC);
		}
	}
}

void AOpYBCharacter::UpdateDeathScreenTime(float TimeLeft)
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AOpYBHUD* HUD = Cast<AOpYBHUD>(PC->GetHUD()))
			{
				HUD->UpdateDeathScreenTime(TimeLeft);
			}
		}
	}
}

void AOpYBCharacter::AddUltCharge()
{
	if (!HasAuthority()) return;

	if (CurrentUltCharge < MaxUltCharge)
	{
		CurrentUltCharge++;
		OnRep_CurrentUltCharge();
	}
}

void AOpYBCharacter::OnRep_CurrentUltCharge()
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (AOpYBHUD* HUD = Cast<AOpYBHUD>(PC->GetHUD()))
			{
				HUD->UpdateUltGauge(CurrentUltCharge, MaxUltCharge);
			}
		}
	}
}

void AOpYBCharacter::ResetUltCharge()
{
	if (HasAuthority())
	{
		CurrentUltCharge = 0;
		OnRep_CurrentUltCharge();
	}
}

void AOpYBCharacter::AddUltimateHit()
{
	if (!HasAuthority()) return;

	if (!bIsUltimateReady)
	{
		UltimateHitCount++;
		if (UltimateHitCount >= 3)
		{
			bIsUltimateReady = true;
		}
	}
}

void AOpYBCharacter::ToggleUltimateAim()
{
	if (IsLocallyControlled())
	{
		if (CanUseUltimate() && !bIsDead)
		{
			bIsAimingUltimate = !bIsAimingUltimate;
		}
	}
}

void AOpYBCharacter::FireUltimate()
{
	if (bIsDead || !CanUseUltimate()) return;

	if (UltimateProjectileClass != nullptr)
	{
		FVector TargetLoc = FVector::ZeroVector;
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FVector WorldLocation, WorldDirection;
			if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				float PlaneZ = GetActorLocation().Z;
				if (FMath::Abs(WorldDirection.Z) > KINDA_SMALL_NUMBER)
				{
					float t = (PlaneZ - WorldLocation.Z) / WorldDirection.Z;
					TargetLoc = WorldLocation + WorldDirection * t;
				}
			}
		}

		if (TargetLoc.IsNearlyZero())
		{
			TargetLoc = GetActorLocation() + GetActorForwardVector() * 1000.0f;
		}

		bIsAimingUltimate = false;
		ServerFireUltimate(TargetLoc);
	}
}

bool AOpYBCharacter::ServerFireUltimate_Validate(FVector TargetLocation)
{
	return true;
}

void AOpYBCharacter::ServerFireUltimate_Implementation(FVector TargetLocation)
{
	if (!CanUseUltimate() || bIsDead) return;

	if (UltimateProjectileClass != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 150.0f + FVector(0.f, 0.f, 100.f);
			FRotator SpawnRotation = GetActorRotation();

			AOpYBUltimateProjectile* Proj = World->SpawnActorDeferred<AOpYBUltimateProjectile>(UltimateProjectileClass, FTransform(SpawnRotation, SpawnLocation), nullptr, this, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (Proj)
			{
				Proj->TargetLocation = TargetLocation;
				Proj->FinishSpawning(FTransform(SpawnRotation, SpawnLocation));
			}

			ResetUltCharge();
		}
	}
}

void AOpYBCharacter::CheckCameraOcclusion()
{
	if (!TopDownCameraComponent) return;

	FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
	FVector CharacterLocation = GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(
		HitResults,
		CameraLocation,
		CharacterLocation,
		ECC_Camera,
		QueryParams
	);

	TSet<const AActor*> CurrentlyHitActors;
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor != this)
		{
			CurrentlyHitActors.Add(HitActor);
		}
	}

	for (const AActor* Actor : HiddenActors)
	{
		if (Actor && !CurrentlyHitActors.Contains(Actor))
		{
			AActor* MutableActor = const_cast<AActor*>(Actor);
			MutableActor->SetActorHiddenInGame(false);
		}
	}

	for (const AActor* Actor : CurrentlyHitActors)
	{
		if (Actor && !HiddenActors.Contains(Actor))
		{
			AActor* MutableActor = const_cast<AActor*>(Actor);
			MutableActor->SetActorHiddenInGame(true);
		}
	}

	HiddenActors = CurrentlyHitActors;
}
