// 프로젝트 세팅의 설명 페이지에 저작권 공지를 채우세요.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpYBProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class OPYB_API AOpYBProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// 기본값을 설정합니다. for this actor's properties
	AOpYBProjectile();

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** Static mesh for visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	/** Function called when the projectile hits something blocking (like walls) */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Function called when the projectile overlaps a pawn (like enemies or self) */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Damage dealt by this projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage = 20.0f;

};
