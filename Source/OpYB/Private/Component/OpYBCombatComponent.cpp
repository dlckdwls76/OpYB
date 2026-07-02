// 에픽게임즈 저작권 소유.

#include "Component/OpYBCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Character/OpYBCharacter.h"
#include "Actor/OpYBProjectile.h"

UOpYBCombatComponent::UOpYBCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UOpYBCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UOpYBCombatComponent, CurrentAmmo);
}

void UOpYBCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentAmmo = MaxAmmo;
	}
	
	LastAmmoCount = CurrentAmmo;
	LocalReloadProgress = 0.0f;

	OnRep_CurrentAmmo();
}

void UOpYBCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentAmmo < MaxAmmo)
	{
		LocalReloadProgress += DeltaTime / ReloadTimePerAmmo;
		if (LocalReloadProgress > 1.0f)
		{
			LocalReloadProgress = 1.0f;
		}
	}
	else
	{
		LocalReloadProgress = 0.0f;
	}

	// Update UI dynamically
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo, LocalReloadProgress);
}

void UOpYBCombatComponent::AttemptShoot(bool bIsUltimate)
{
	AOpYBCharacter* Character = Cast<AOpYBCharacter>(GetOwner());
	if (!Character || Character->bIsDead) return;

	if (Character->bIsAimingUltimate || bIsUltimate)
	{
		Character->FireUltimate();
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (Character->IsRolling()) return;

	if (CurrentAmmo > 0 && CurrentTime - LastFireTime >= FireRate)
	{
		LastFireTime = CurrentTime;

		if (!GetOwner()->HasAuthority())
		{
			CurrentAmmo--;
			OnRep_CurrentAmmo();
		}

		FVector SpawnLocation = Character->GetActorLocation() + FVector(0, 0, 20.0f);
		FRotator SpawnRotation = Character->GetActorRotation();
		
		ServerShoot(SpawnLocation, SpawnRotation, bIsUltimate);
	}
}

bool UOpYBCombatComponent::ServerShoot_Validate(FVector SpawnLocation, FRotator SpawnRotation, bool bIsUltimate)
{
	return true;
}

void UOpYBCombatComponent::ServerShoot_Implementation(FVector SpawnLocation, FRotator SpawnRotation, bool bIsUltimate)
{
	if (ProjectileClass)
	{
		FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		AOpYBProjectile* Proj = GetWorld()->SpawnActorDeferred<AOpYBProjectile>(ProjectileClass, SpawnTransform, GetOwner(), Cast<APawn>(GetOwner()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		if (Proj)
		{
			Proj->FinishSpawning(SpawnTransform);

			if (CurrentAmmo > 0)
			{
				CurrentAmmo--;
				OnRep_CurrentAmmo();

				if (!GetOwner()->GetWorldTimerManager().IsTimerActive(RechargeTimerHandle))
				{
					GetOwner()->GetWorldTimerManager().SetTimer(RechargeTimerHandle, this, &UOpYBCombatComponent::RechargeAmmo, ReloadTimePerAmmo, true);
				}
			}
		}
	}
}

void UOpYBCombatComponent::RechargeAmmo()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (CurrentAmmo < MaxAmmo)
		{
			CurrentAmmo++;
			OnRep_CurrentAmmo(); 
		}

		if (CurrentAmmo >= MaxAmmo)
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(RechargeTimerHandle);
		}
	}
}

void UOpYBCombatComponent::OnRep_CurrentAmmo()
{
	if (CurrentAmmo > LastAmmoCount)
	{
		LocalReloadProgress = 0.0f;
	}

	LastAmmoCount = CurrentAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo, LocalReloadProgress);
}

void UOpYBCombatComponent::ResetAmmo()
{
	CurrentAmmo = MaxAmmo;
	LastAmmoCount = MaxAmmo;
	LocalReloadProgress = 0.0f;
	OnRep_CurrentAmmo();
}
