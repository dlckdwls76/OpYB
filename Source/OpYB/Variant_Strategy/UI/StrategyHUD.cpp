// 에픽게임즈 저작권 소유.


#include "StrategyHUD.h"
#include "StrategyUnit.h"
#include "StrategyPlayerController.h"
#include "StrategyUI.h"

void AStrategyHUD::BeginPlay()
{
	Super::BeginPlay();

	// UI 위젯 스폰
	UIWidget = CreateWidget<UStrategyUI>(GetOwningPlayerController(), UIWidgetClass);
	check(UIWidget);

	// 화면에 UI 위젯 추가
	UIWidget->AddToViewport(0);
}

void AStrategyHUD::DragSelectUpdate(FVector2D Start, FVector2D WidthAndHeight, FVector2D CurrentPosition, bool bDraw)
{
	// 선택 박스 데이터 복사
	bDrawBox = bDraw;
	BoxStart = Start;
	BoxSize = WidthAndHeight;
	BoxCurrentPosition = CurrentPosition;

}

void AStrategyHUD::DrawHUD()
{
	// 모든 디버그 정보 등을 그립니다.
	Super::DrawHUD();

	// 유효한 플레이어 컨트롤러가 있는지 확인
	if (AStrategyPlayerController* PC = Cast<AStrategyPlayerController>(GetOwningPlayerController()))
	{
		// 선택 박스 그리기
		if (bDrawBox)
		{
			DrawRect(SelectionBoxColor, BoxStart.X, BoxStart.Y, BoxSize.X, BoxSize.Y);

			// 선택 박스 내의 모든 유닛 가져오기
			TArray<AStrategyUnit*> BoxedUnits;
			GetActorsInSelectionRectangle(BoxStart, BoxCurrentPosition, BoxedUnits, true);

			// 플레이어 컨트롤러의 유닛 선택 업데이트
			PC->DragSelectUnits(BoxedUnits);
		}

		// 현재 선택된 유닛 가져오기
		TArray<AStrategyUnit*> SelectedUnits = PC->GetSelectedUnits();

		// UI 위젯의 선택 개수 업데이트
		UIWidget->SetSelectedUnitsCount(SelectedUnits.Num());

		// 선택한 각 유닛 처리
		for (AStrategyUnit* CurrentUnit : SelectedUnits)
		{
			if (IsValid(CurrentUnit))
			{
				// 유닛의 위치를 화면 좌표로 투영
				FVector2D ScreenCoords;

				if (PC->ProjectWorldLocationToScreen(CurrentUnit->GetActorLocation(), ScreenCoords, true))
				{
					// 유닛 근처에 선택 문자열 그리기
					const FString SelectionString = "Selected";
					DrawText(SelectionString, FColor::White, ScreenCoords.X - 25.0f, ScreenCoords.Y + 25.0f, nullptr, 1.5f);
				}
			}
			
		}
	}

}
