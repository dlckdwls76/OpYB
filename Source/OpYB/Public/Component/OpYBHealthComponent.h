// 에픽게임즈 저작권 소유.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpYBHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawnSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRespawnTimeUpdatedSignature, float, TimeLeft);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPYB_API UOpYBHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOpYBHealthComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Stats")
	bool bIsDead;

	UFUNCTION()
	void OnRep_IsDead();

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float RespawnTimeLeft;

	FTimerHandle RespawnTimerHandle;

	float HandleTakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	void Die();

	void Respawn();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathSignature OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRespawnSignature OnRespawn;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRespawnTimeUpdatedSignature OnRespawnTimeUpdated;
};
