#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpYBUltimateProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class OPYB_API AOpYBUltimateProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpYBUltimateProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere,	 BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** Static mesh for visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	/** Damage dealt by the ultimate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageAmount = 50.0f;

	/** Knockback strength applied to enemies hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float KnockbackStrength = 500.0f;

	/** Function called when the projectile overlaps a pawn (like enemies) */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
