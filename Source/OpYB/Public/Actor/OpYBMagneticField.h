#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpYBMagneticField.generated.h"

class USphereComponent;
class UPostProcessComponent;

UCLASS()
class OPYB_API AOpYBMagneticField : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpYBMagneticField();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Visual representation of the magnetic field
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// Post process component for visual indication outside the field
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPostProcessComponent> PostProcessComp;

	// Current radius of the safe zone
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float CurrentRadius = 5000.f;

	// Minimum radius to which the field will shrink
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float MinRadius = 500.f;

	// How much the radius shrinks per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float ShrinkRate = 50.f;

	// Damage dealt per tick to characters outside the safe zone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float DamagePerTick = 5.f;

	// Interval between damage ticks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float DamageInterval = 1.0f;

	// Network Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FTimerHandle DamageTimerHandle;

	// Function to apply damage to players outside the zone
	void ApplyDamageToPlayersOutside();
};
