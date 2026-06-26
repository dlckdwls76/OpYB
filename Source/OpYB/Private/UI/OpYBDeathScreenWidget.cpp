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
