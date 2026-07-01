// 에픽게임즈 저작권 소유.

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



