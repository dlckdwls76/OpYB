#include "UI/OpYBHUD.h"
#include "UI/OpYBAimCursorWidget.h"
#include "UI/OpYBPlayerCountWidget.h"
#include "UI/OpYBDeathScreenWidget.h"
#include "UI/OpYBUltimateWidget.h"

void AOpYBHUD::BeginPlay()
{
	Super::BeginPlay();

	if (AimCursorClass)
	{
		AimCursorInstance = CreateWidget<UOpYBAimCursorWidget>(GetWorld(), AimCursorClass);
		if (AimCursorInstance)
		{
			AimCursorInstance->AddToViewport(100);
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("[오류] HUD에 Aim Cursor Class가 지정되지 않았습니다!"));
	}

	if (PlayerCountClass)
	{
		PlayerCountInstance = CreateWidget<UOpYBPlayerCountWidget>(GetWorld(), PlayerCountClass);
		if (PlayerCountInstance)
		{
			PlayerCountInstance->AddToViewport();
		}
	}

	if (UltimateGaugeClass)
	{
		UltimateGaugeInstance = CreateWidget<UOpYBUltimateWidget>(GetWorld(), UltimateGaugeClass);
		if (UltimateGaugeInstance)
		{
			UltimateGaugeInstance->AddToViewport();
		}
	}
}

void AOpYBHUD::ShowDeathScreen()
{
	// 전투 UI 숨기기
	if (AimCursorInstance) AimCursorInstance->SetVisibility(ESlateVisibility::Hidden);
	if (UltimateGaugeInstance) UltimateGaugeInstance->SetVisibility(ESlateVisibility::Hidden);

	if (DeathScreenClass && !DeathScreenInstance)
	{
		DeathScreenInstance = CreateWidget<UOpYBDeathScreenWidget>(GetWorld(), DeathScreenClass);
		if (DeathScreenInstance)
		{
			DeathScreenInstance->AddToViewport(50);
		}
	}
	else if (!DeathScreenClass)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, TEXT("[오류] HUD에 Death Screen Class가 지정되지 않았습니다!"));
	}
}

void AOpYBHUD::HideDeathScreen()
{
	// 전투 UI 다시 표시하기
	if (AimCursorInstance) AimCursorInstance->SetVisibility(ESlateVisibility::Visible);
	if (UltimateGaugeInstance) UltimateGaugeInstance->SetVisibility(ESlateVisibility::Visible);

	if (DeathScreenInstance)
	{
		DeathScreenInstance->RemoveFromParent();
		DeathScreenInstance = nullptr;
	}
}

void AOpYBHUD::PlayShootAnimation()
{
	if (AimCursorInstance)
	{
		AimCursorInstance->PlayShootAnimation();
	}
}

void AOpYBHUD::UpdateDeathScreenTime(float TimeLeft)
{
	if (DeathScreenInstance)
	{
		DeathScreenInstance->UpdateTimeLeft(FMath::CeilToInt(TimeLeft));
	}
}

void AOpYBHUD::UpdateUltGauge(int32 CurrentCharge, int32 MaxCharge)
{
	if (UltimateGaugeInstance)
	{
		UltimateGaugeInstance->UpdateUltGauge(CurrentCharge, MaxCharge);
	}
}

void AOpYBHUD::SetAimMode(bool bIsUltMode)
{
	if (AimCursorInstance)
	{
		AimCursorInstance->SetAimMode(bIsUltMode);
	}
}
