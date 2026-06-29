// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/OpYBGameMode.h"
#include "Game/OpYBGameState.h"
#include "UI/OpYBHUD.h"

AOpYBGameMode::AOpYBGameMode()
{
	GameStateClass = AOpYBGameState::StaticClass();
	HUDClass = AOpYBHUD::StaticClass();
}