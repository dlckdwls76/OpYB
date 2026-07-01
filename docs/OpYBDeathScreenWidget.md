# OpYBDeathScreenWidget

## OpYBDeathScreenWidget.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpYBDeathScreenWidget.generated.h"

class UTextBlock;

/**
 * Base class for the Death Screen UI
 */
UCLASS(Abstract)
class OPYB_API UOpYBDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Text block to display remaining respawn time. Must be named 'TimeText' in Blueprint */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

	/** Updates the displayed text */
	void UpdateTimeLeft(int32 TimeLeft);
};
```

## OpYBDeathScreenWidget.cpp
```cpp
#include "UI/OpYBDeathScreenWidget.h"
#include "Components/TextBlock.h"

void UOpYBDeathScreenWidget::UpdateTimeLeft(int32 TimeLeft)
{
	if (TimeText)
	{
		FString TextStr = FString::Printf(TEXT("남은시간 : %d"), TimeLeft);
		TimeText->SetText(FText::FromString(TextStr));
	}
}
```
