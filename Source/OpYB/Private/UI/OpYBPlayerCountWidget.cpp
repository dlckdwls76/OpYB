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
