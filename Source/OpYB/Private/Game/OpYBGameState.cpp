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
