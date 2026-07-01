// 에픽게임즈 저작권 소유.

#include "Game/OpYBGameMode.h"
#include "Game/OpYBGameState.h"
#include "UI/OpYBHUD.h"

AOpYBGameMode::AOpYBGameMode()
{
	GameStateClass = AOpYBGameState::StaticClass();
	HUDClass = AOpYBHUD::StaticClass();
}