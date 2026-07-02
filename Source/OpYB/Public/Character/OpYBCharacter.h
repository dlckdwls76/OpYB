// 에픽게임즈 저작권 소유.

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
class UOpYBHealthComponent;
class UOpYBCombatComponent;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> OverheadUIComponent;

	/** Health Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UOpYBHealthComponent> HealthComponent;

	/** Combat Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UOpYBCombatComponent> CombatComponent;

public:

	/** Constructor */
	AOpYBCharacter();

	/** Initialization */
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Network Replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns the camera component **/
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }

	/** Returns the Camera Boom component **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }

	/** Returns Health Component */
	UOpYBHealthComponent* GetHealthComponent() const { return HealthComponent.Get(); }

	/** Returns Combat Component */
	UOpYBCombatComponent* GetCombatComponent() const { return CombatComponent.Get(); }

	/** 구르기 몽타주 (Spacebar) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	TObjectPtr<UAnimMontage> RollMontage;

	/** Play the roll montage */
	void PlayRollMontage();

protected:
	/** 카메라에 가려져 숨김 처리된 액터들의 목록 */
	TSet<const AActor*> HiddenActors;

	/** 카메라 가림 검사(Occlusion) 타이머 핸들 */
	FTimerHandle CameraOcclusionTimerHandle;

	/** 매 프레임 카메라 레이캐스트를 쏴서 장애물을 투명화/복구하는 함수 */
	void CheckCameraOcclusion();

public:

	/** Returns true if character is currently rolling */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsRolling() const;

	// ==========================================
	// UI Delegate Handlers (Health & Death)
	// ==========================================
	
	UFUNCTION()
	void UpdateHealthUI(float InCurrentHealth, float InMaxHealth);

	UFUNCTION()
	void UpdateAmmoUI(int32 InCurrentAmmo, int32 InMaxAmmo, float InReloadProgress);

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleRespawn();

	UFUNCTION()
	void UpdateDeathScreenTime(float TimeLeft);
	
	// 편의상 bIsDead 상태 플래그 (다른 컴포넌트 접근용)
	bool bIsDead = false;

	/** 궁극기 게이지 (0~3) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentUltCharge, VisibleAnywhere, BlueprintReadOnly, Category = "Ultimate")
	int32 CurrentUltCharge;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ultimate")
	int32 MaxUltCharge;

	UFUNCTION()
	void OnRep_CurrentUltCharge();

	/** 궁극기 게이지 추가 */
	void AddUltCharge();

	/** 궁극기 사용 가능 여부 확인 */
	bool CanUseUltimate() const { return CurrentUltCharge >= MaxUltCharge; }

	/** 사망 시 게이지 초기화 */
	void ResetUltCharge();

	/** Take Damage Function */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// ==========================================
	// 궁극기 관련 기능
	// ==========================================

	/** 궁극기 발사에 사용할 투사체 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Ultimate")
	TSubclassOf<class AOpYBUltimateProjectile> UltimateProjectileClass;

	/** 궁극기 사용을 위한 현재 명중 횟수 (최대 3) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat|Ultimate")
	int32 UltimateHitCount = 0;

	/** 궁극기 사용 가능 여부 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat|Ultimate")
	bool bIsUltimateReady = false;

	/** 궁극기 조준 상태 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Ultimate")
	bool bIsAimingUltimate = false;

	/** 적 명중 시 횟수 증가 (서버 전용) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ultimate")
	void AddUltimateHit();

	/** R키 눌렀을 때 조준 상태 토글 (로컬) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ultimate")
	void ToggleUltimateAim();

	/** 궁극기 발사 시도 (로컬) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ultimate")
	void FireUltimate();

	/** 궁극기 발사 서버 RPC */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireUltimate(FVector TargetLocation);

};
