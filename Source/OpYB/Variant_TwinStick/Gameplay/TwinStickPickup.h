// 에픽게임즈 저작권 소유.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TwinStickPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  A simple pickup for a Twin Stick Shooter game
 */
UCLASS(abstract)
class ATwinStickPickup : public AActor
{
	GENERATED_BODY()
	
	/** Pickup collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionSphere;

	/** Provides visual representation for the pickup */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

public:	

	/** Constructor */
	ATwinStickPickup();

	/** Collision handling */
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
