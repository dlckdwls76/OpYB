// 에픽게임즈 저작권 소유.

#include "Component/OpYBHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Game/OpYBGameState.h"
#include "Character/OpYBCharacter.h"

UOpYBHealthComponent::UOpYBHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UOpYBHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UOpYBHealthComponent, CurrentHealth);
	DOREPLIFETIME(UOpYBHealthComponent, bIsDead);
}

void UOpYBHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
	
	bIsDead = false;
	RespawnTimeLeft = 0.0f;

	OnRep_CurrentHealth();
	OnRep_IsDead();
}

void UOpYBHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (GetOwner() && GetOwner()->HasAuthority() && !bIsDead)
	{
		if (AOpYBGameState* GS = GetWorld()->GetGameState<AOpYBGameState>())
		{
			GS->RemoveAlivePlayer();
		}
	}
}

void UOpYBHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead)
	{
		if (GetOwner() && Cast<APawn>(GetOwner())->IsLocallyControlled())
		{
			RespawnTimeLeft -= DeltaTime;
			if (RespawnTimeLeft < 0.0f) RespawnTimeLeft = 0.0f;
			
			OnRespawnTimeUpdated.Broadcast(RespawnTimeLeft);
		}
	}
}

void UOpYBHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UOpYBHealthComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		if (GetOwner() && Cast<APawn>(GetOwner())->IsLocallyControlled())
		{
			RespawnTimeLeft = 10.0f;
		}
		OnDeath.Broadcast();
	}
	else
	{
		OnRespawn.Broadcast();
	}
}

float UOpYBHealthComponent::HandleTakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth -= DamageAmount;

		if (EventInstigator)
		{
			if (AOpYBCharacter* Attacker = Cast<AOpYBCharacter>(EventInstigator->GetPawn()))
			{
				if (Attacker != GetOwner())
				{
					Attacker->AddUltCharge();
				}
			}
		}

		if (CurrentHealth <= 0.0f)
		{
			CurrentHealth = 0.0f;
			Die();
		}
		OnRep_CurrentHealth();
	}

	return DamageAmount;
}

void UOpYBHealthComponent::Die()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	bIsDead = true;
	OnRep_IsDead();
	
	if (AOpYBCharacter* Character = Cast<AOpYBCharacter>(GetOwner()))
	{
		Character->ResetUltCharge();
	}

	if (AOpYBGameState* GS = GetWorld()->GetGameState<AOpYBGameState>())
	{
		GS->RemoveAlivePlayer();
	}

	GetOwner()->GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &UOpYBHealthComponent::Respawn, 10.0f, false);
}

void UOpYBHealthComponent::Respawn()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	if (PlayerStarts.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		AActor* SelectedStart = PlayerStarts[RandomIndex];
		GetOwner()->SetActorLocationAndRotation(SelectedStart->GetActorLocation(), SelectedStart->GetActorRotation());
	}
	
	CurrentHealth = MaxHealth;
	
	OnRep_CurrentHealth();

	bIsDead = false;
	OnRep_IsDead();

	if (AOpYBGameState* GS = GetWorld()->GetGameState<AOpYBGameState>())
	{
		GS->AddAlivePlayer();
	}
}
