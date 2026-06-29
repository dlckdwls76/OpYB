#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpYBInvisibleWall.generated.h"

class UBoxComponent;

UCLASS()
class OPYB_API AOpYBInvisibleWall : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOpYBInvisibleWall();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Box collision component to block movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;
};
