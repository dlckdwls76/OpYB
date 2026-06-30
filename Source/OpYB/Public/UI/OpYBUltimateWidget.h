#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpYBUltimateWidget.generated.h"

class UProgressBar;

/**
 *  Base class for the Ultimate Gauge UI
 */
UCLASS(Abstract)
class OPYB_API UOpYBUltimateWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** Updates the 3 slots of the ultimate gauge */
	UFUNCTION(BlueprintCallable, Category = "Ultimate UI")
	void UpdateUltGauge(int32 CurrentCharge, int32 MaxCharge);

	/** Blueprint Event to trigger blinking when full */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ultimate UI")
	void PlayUltReadyAnimation();

	/** Blueprint Event to stop blinking when used */
	UFUNCTION(BlueprintImplementableEvent, Category = "Ultimate UI")
	void StopUltReadyAnimation();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> UltSlot1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> UltSlot2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> UltSlot3;
};
