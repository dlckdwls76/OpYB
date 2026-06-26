#include "UI/OpYBAimCursorWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"

void UOpYBAimCursorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// 위젯의 중심이 실제 좌표가 되도록 정렬 (마우스 클릭 지점)
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
}

void UOpYBAimCursorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 1. 크기를 원래 크기(1.0)로 서서히 되돌림 (보간)
	CurrentScale = FMath::FInterpTo(CurrentScale, 1.0f, InDeltaTime, 15.0f);
	SetRenderScale(FVector2D(CurrentScale, CurrentScale));

	// 2. 마우스 위치를 계속 따라가기
	if (APlayerController* PC = GetOwningPlayer())
	{
		float MouseX, MouseY;
		// 마우스 좌표 추적에 성공했을 때만 위치 갱신 (클릭 시 (0,0)으로 튀는 현상 방지)
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			SetPositionInViewport(FVector2D(MouseX, MouseY));
		}
	}
}

void UOpYBAimCursorWidget::PlayShootAnimation()
{
	// 사격 시 크기를 1.5배로 뻥튀기
	CurrentScale = 1.5f; 
}
