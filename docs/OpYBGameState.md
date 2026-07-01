# OpYBGameState

## OpYBGameState.h
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OpYBGameState.generated.h"

/**
 * 
 */
UCLASS()
class OPYB_API AOpYBGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AOpYBGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Adds to the alive player count (Server Only) */
	void AddAlivePlayer();

	/** Removes from the alive player count (Server Only) */
	void RemoveAlivePlayer();

	/** Gets the current alive player count */
	int32 GetAlivePlayerCount() const { return AlivePlayerCount; }

protected:
	UPROPERTY(Replicated)
	int32 AlivePlayerCount;
};
```

## OpYBGameState.cpp
```cpp
#include "Game/OpYBGameState.h"
#include "Net/UnrealNetwork.h"

AOpYBGameState::AOpYBGameState()
{
	AlivePlayerCount = 0;
}

void AOpYBGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOpYBGameState, AlivePlayerCount);
}

void AOpYBGameState::AddAlivePlayer()
{
	if (HasAuthority())
	{
		AlivePlayerCount++;
	}
}

void AOpYBGameState::RemoveAlivePlayer()
{
	if (HasAuthority())
	{
		AlivePlayerCount--;
		if (AlivePlayerCount < 0)
		{
			AlivePlayerCount = 0;
		}
	}
}
```
