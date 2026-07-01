#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpYBMagneticField.generated.h"

class USphereComponent;
class UPostProcessComponent;

UCLASS()
class OPYB_API AOpYBMagneticField : public AActor
{
	GENERATED_BODY()
	
public:	
	// 기본값을 설정합니다. for this actor's properties
	AOpYBMagneticField();

protected:
	// 게임이 시작되거나 스폰될 때 호출됩니다.
	virtual void BeginPlay() override;

public:	
	// 매 프레임 호출됩니다.
	virtual void Tick(float DeltaTime) override;

	// 자기장의 시각적 표현
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// 자기장 외부 시각적 표시를 위한 포스트 프로세스 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPostProcessComponent> PostProcessComp;

	// 현재 안전 구역 반지름
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float CurrentRadius = 5000.f;

	// 자기장이 축소되는 최소 반지름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float MinRadius = 500.f;

	// 초당 반지름 축소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float ShrinkRate = 50.f;

	// 안전 구역 밖의 캐릭터에게 틱당 가해지는 피해량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float DamagePerTick = 5.f;

	// 피해 틱 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnetic Field")
	float DamageInterval = 1.0f;

	// 네트워크 리플리케이션
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FTimerHandle DamageTimerHandle;

	// 구역 밖의 플레이어에게 피해를 입히는 함수
	void ApplyDamageToPlayersOutside();
};
