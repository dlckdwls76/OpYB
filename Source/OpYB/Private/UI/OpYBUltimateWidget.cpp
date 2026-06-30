#include "UI/OpYBUltimateWidget.h"
#include "Components/ProgressBar.h"

void UOpYBUltimateWidget::UpdateUltGauge(int32 CurrentCharge, int32 MaxCharge)
{
	// 탄창 3칸처럼 1칸씩 채워지는 로직
	if (UltSlot1) UltSlot1->SetPercent(CurrentCharge >= 1 ? 1.0f : 0.0f);
	if (UltSlot2) UltSlot2->SetPercent(CurrentCharge >= 2 ? 1.0f : 0.0f);
	if (UltSlot3) UltSlot3->SetPercent(CurrentCharge >= 3 ? 1.0f : 0.0f);

	if (CurrentCharge >= MaxCharge)
	{
		PlayUltReadyAnimation();
	}
	else
	{
		StopUltReadyAnimation();
	}
}
