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
	// 기본값을 설정합니다. for this actor's properties
	AOpYBUltimateProjectile();

protected:
	// 게임이 시작되거나 스폰될 때 호출됩니다.
	virtual void BeginPlay() override;

public:	
	// 매 프레임 호출됩니다.
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

	/** Explosion radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ExplosionRadius = 500.0f;

	/** Knockback strength applied to enemies hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float KnockbackStrength = 2000.0f;

	/** Explosion particle effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<class UParticleSystem> ExplosionEffect;

	/** Danger zone decal */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Components")
	TObjectPtr<class UDecalComponent> DangerZoneDecal;

	/** Target location for the projectile to land */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Combat", meta = (ExposeOnSpawn="true"))
	FVector TargetLocation;

	/** Function called when the projectile hits something (floor/wall) */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Function called when the projectile overlaps a pawn (like enemies) */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Explosion handling - replicated on all clients */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastExplode();

	/** Destroy projectile on all clients */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDestroy();

};
