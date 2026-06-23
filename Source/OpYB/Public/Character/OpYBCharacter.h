// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OpYBCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class AOpYBProjectile;

/**
 *  A controllable top-down perspective character
 */
UCLASS() 
class AOpYBCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

public:

	/** Constructor */
	AOpYBCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns the camera component **/
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }

	/** Returns the Camera Boom component **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }

	/** 구르기 몽타주 (Spacebar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimMontage> RollMontage;

	/** Play the roll montage */
	void PlayRollMontage();

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AOpYBProjectile> ProjectileClass;

	/** 연사 속도 (초 단위). 기본값 0.1초 (1초에 10발) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float FireRate;

	/** 마지막으로 발사한 시간 기록용 */
	float LastFireTime;

	/** Attempts to shoot (called locally) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttemptShoot();

	/** Server RPC for shooting */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShoot(FVector SpawnLocation, FRotator SpawnRotation);

};
