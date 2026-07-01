// 에픽게임즈 저작권 소유.


#include "StrategyUI.h"

void UStrategyUI::SetSelectedUnitsCount(int32 Count)
{
	// 다른 카운트입니까?
	bool bChanged = SelectedUnitCount != Count;

	// 카운터 업데이트
	SelectedUnitCount = Count;

	// 카운트가 변경되면 BP 핸들러 호출
	if (bChanged)
	{
		BP_UpdateUnitsCount();
	}
}
