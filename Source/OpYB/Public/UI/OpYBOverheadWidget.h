// 에픽게임즈 저작권 소유.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "OpYBOverheadWidget.generated.h"

/**
 *  Base class for the overhead UI (Health and Ammo)
 */
UCLASS(Abstract)
class OPYB_API UOpYBOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** The progress bar widget for health. Must be named exactly 'Healthbar' in Blueprint */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Healthbar;

	/** Update health visuals */
	UFUNCTION(BlueprintCallable, Category = "Overhead UI")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	/** The progress bars representing ammo. Must be named exactly Ammo1, Ammo2, Ammo3 in Blueprint */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Ammo1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Ammo2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> Ammo3;

	/** Update ammo visuals */
	UFUNCTION(BlueprintCallable, Category = "Overhead UI")
	void UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo, float ReloadProgress);

};
