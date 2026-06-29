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
