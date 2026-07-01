# OpYBOverheadWidget

## OpYBOverheadWidget.h
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

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
```

## OpYBOverheadWidget.cpp
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/OpYBOverheadWidget.h"
#include "Components/ProgressBar.h"

void UOpYBOverheadWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	if (Healthbar && MaxHealth > 0.0f)
	{
		Healthbar->SetPercent(CurrentHealth / MaxHealth);
	}
}

void UOpYBOverheadWidget::UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo, float ReloadProgress)
{
	// 탄약이 완전히 찼을 때의 설정
	auto FillAmmoBar = [](UProgressBar* Bar, float Progress)
	{
		if (Bar)
		{
			Bar->SetPercent(Progress);
			Bar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.4f, 0.0f, 1.0f)); // 주황색
		}
	};

	if (Ammo1)
	{
		if (CurrentAmmo >= 1) FillAmmoBar(Ammo1, 1.0f);
		else if (CurrentAmmo == 0) FillAmmoBar(Ammo1, ReloadProgress);
		else FillAmmoBar(Ammo1, 0.0f);
	}

	if (Ammo2)
	{
		if (CurrentAmmo >= 2) FillAmmoBar(Ammo2, 1.0f);
		else if (CurrentAmmo == 1) FillAmmoBar(Ammo2, ReloadProgress);
		else FillAmmoBar(Ammo2, 0.0f);
	}

	if (Ammo3)
	{
		if (CurrentAmmo >= 3) FillAmmoBar(Ammo3, 1.0f);
		else if (CurrentAmmo == 2) FillAmmoBar(Ammo3, ReloadProgress);
		else FillAmmoBar(Ammo3, 0.0f);
	}
}
```
