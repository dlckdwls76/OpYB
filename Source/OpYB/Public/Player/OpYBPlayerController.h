// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OpYBPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UNiagaraSystem;

/**
 *  Player controller for a top-down perspective game.
 */
UCLASS()
class AOpYBPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** FX Class that we will spawn when clicking (Kept for blueprint compatibility) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UNiagaraSystem> FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	/** Move Forward Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveForwardAction;

	/** Move Right Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveRightAction;

	/** Shoot Input Action (Left Click) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	/** Dynamic Mapping Context */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> DynamicMappingContext;

public:

	/** Constructor */
	AOpYBPlayerController();

	/** BeginPlay */
	virtual void BeginPlay() override;

	/** Update loop */
	virtual void Tick(float DeltaTime) override;

protected:

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Called for movement input */
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);

	/** Called for shoot input */
	void Shoot();

	/** Server RPC for syncing rotation */
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSetPawnRotation(FRotator NewRotation);

};
