#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpYBAimCursorWidget.generated.h"

/**
 * Custom dynamic aim cursor widget
 */
UCLASS(Abstract)
class OPYB_API UOpYBAimCursorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Called when the player shoots */
	void PlayShootAnimation();

protected:
	/** Current animated scale of the crosshair */
	float CurrentScale = 1.0f;
};
