# OpYBGameMode

## OpYBGameMode.h
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OpYBGameMode.generated.h"

/**
 *  Simple Game Mode for a top-down perspective game
 *  Sets the default gameplay framework classes
 *  Check the Blueprint derived class for the set values
 */
UCLASS()
class AOpYBGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	AOpYBGameMode();
};
```

## OpYBGameMode.cpp
```cpp
// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/OpYBGameMode.h"
#include "Game/OpYBGameState.h"
#include "UI/OpYBHUD.h"

AOpYBGameMode::AOpYBGameMode()
{
	GameStateClass = AOpYBGameState::StaticClass();
	HUDClass = AOpYBHUD::StaticClass();
}
```
