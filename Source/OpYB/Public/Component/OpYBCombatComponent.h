// 에픽게임즈 저작권 소유.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpYBCombatComponent.generated.h"

class AOpYBProjectile;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChangedSignature, int32, CurrentAmmo, int32, MaxAmmo, float, ReloadProgress);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPYB_API UOpYBCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOpYBCombatComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AOpYBProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Shooting")
	float FireRate = 0.1f;

	float LastFireTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 MaxAmmo = 3;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, BlueprintReadOnly, Category = "Stats")
	int32 CurrentAmmo;

	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float ReloadTimePerAmmo = 3.0f;

	float LocalReloadProgress = 0.0f;
	int32 LastAmmoCount = 3;

	FTimerHandle RechargeTimerHandle;
	void RechargeAmmo();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttemptShoot(bool bIsUltimate);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShoot(FVector SpawnLocation, FRotator SpawnRotation, bool bIsUltimate);

	void ResetAmmo();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAmmoChangedSignature OnAmmoChanged;
};
