// 에픽게임즈 저작권 소유.

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
	TObjectPtr<UInputMappingContext> DefaultMapingContext;
	
	/** Move Forward Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveForwardAction;

	/** Move Right Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveRightAction;

	/** Shoot Input Action (Left Click) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootAction;

	/** 궁극기 토글 입력 액션 (R키) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UltimateAction;

	/** Roll Input Action (Spacebar) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RollAction;

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

	/** Input handler for shooting */
	void Shoot();

	/** 궁극기 토글 함수 */
	void ToggleUltimate();

	/** 궁극기 모드 켜져있는지 여부 */
	bool bIsUltReadyMode = false;

	/** Called for ultimate action input */
	void OnUltimateAction();

	/** Called for roll input */
	void DoRoll();

	/** Server RPC for syncing rotation */
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSetPawnRotation(FRotator NewRotation);
	
	/** 최근에 전송한 회전값 캐싱 (네트워크 대역폭 최적화용) */
	FRotator LastSentRotation = FRotator::ZeroRotator;

	/** 마지막으로 회전값을 전송한 이후 흐른 시간 */
	float TimeSinceLastRotationSync = 0.0f;
};
