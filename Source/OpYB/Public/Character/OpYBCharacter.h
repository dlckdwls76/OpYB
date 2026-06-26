// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "OpYBCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class AOpYBProjectile;
class UWidgetComponent;
class UOpYBDeathScreenWidget;

/**
 *  A controllable top-down perspective character
 */
UCLASS(Blueprintable) 
class AOpYBCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Overhead UI Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> OverheadUIComponent;

public:

	/** Constructor */
	AOpYBCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Network Replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns the camera component **/
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }

	/** Returns the Camera Boom component **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }

	/** 구르기 몽타주 (Spacebar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimMontage> RollMontage;

	/** Play the roll montage */
	void PlayRollMontage();

	/** Returns true if character is currently rolling */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsRolling() const;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AOpYBProjectile> ProjectileClass;

	/** 연사 속도 (초 단위). 기본값 0.1초 (1초에 10발) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Shooting")
	float FireRate;

	/** 마지막으로 발사한 시간 기록용 */
	float LastFireTime;
	
	/** Attempts to shoot (called locally) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttemptShoot();

	/** Server RPC for shooting */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShoot(FVector SpawnLocation, FRotator SpawnRotation);

	/** Maximum Health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 100.0f;

	/** Current Health */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Maximum Ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	int32 MaxAmmo = 3;

	/** Current Ammo */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo, BlueprintReadOnly, Category = "Stats")
	int32 CurrentAmmo;

	/** 현재 장탄수 변경 시 호출 */
	UFUNCTION()
	void OnRep_CurrentAmmo();

	/** 사망 여부 */
	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadOnly, Category = "Stats")
	bool bIsDead;

	UFUNCTION()
	void OnRep_IsDead();

	/** 데스 스크린 UI 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UOpYBDeathScreenWidget> DeathScreenClass;

	/** 생성된 데스 스크린 인스턴스 */
	UPROPERTY()
	TObjectPtr<UOpYBDeathScreenWidget> DeathScreenInstance;

	/** 남은 부활 시간 (클라이언트 UI 표시용) */
	float RespawnTimeLeft;

	/** 부활 타이머 (서버용) */
	FTimerHandle RespawnTimerHandle;

	/** 사망 처리 (서버 전용) */
	void Die();

	/** 부활 처리 (서버 전용) */
	void Respawn();

	/** Take Damage Function */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** 한 발 충전에 걸리는 시간 (초 단위) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float ReloadTimePerAmmo = 1.0f;

	/** 로컬 장전 시각화용 진행도 (0.0 ~ 1.0) */
	float LocalReloadProgress = 0.0f;

	/** 이전 탄약 개수 기억용 (장전 완료 감지) */
	int32 LastAmmoCount = 3;

	/** 탄약 충전용 타이머 핸들 (서버용) */
	FTimerHandle RechargeTimerHandle;

	/** 한 발 충전 완료 시 호출될 함수 (서버용) */
	void RechargeAmmo();

};
