# OpYBPlayerCountWidget

## OpYBPlayerCountWidget.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpYBPlayerCountWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS(Abstract)
class OPYB_API UOpYBPlayerCountWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;
};
```

## OpYBPlayerCountWidget.cpp
```cpp
#include "UI/OpYBPlayerCountWidget.h"
#include "Components/TextBlock.h"
#include "Game/OpYBGameState.h"

void UOpYBPlayerCountWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PlayerCountText)
	{
		if (AOpYBGameState* GS = GetWorld()->GetGameState<AOpYBGameState>())
		{
			FString TextStr = FString::Printf(TEXT("남은 인원 : %d"), GS->GetAlivePlayerCount());
			PlayerCountText->SetText(FText::FromString(TextStr));
		}
	}
}
```
