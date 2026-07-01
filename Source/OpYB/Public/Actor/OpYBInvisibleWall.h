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
	// 기본값을 설정합니다. for this actor's properties
	AOpYBInvisibleWall();

protected:
	// 게임이 시작되거나 스폰될 때 호출됩니다.
	virtual void BeginPlay() override;

public:	
	// 이동을 차단하는 박스 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionBox;
};
