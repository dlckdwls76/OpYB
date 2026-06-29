#include "UI/OpYBHUD.h"
#include "UI/OpYBAimCursorWidget.h"
#include "UI/OpYBPlayerCountWidget.h"
#include "UI/OpYBDeathScreenWidget.h"

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
}

void AOpYBHUD::ShowDeathScreen()
{
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
