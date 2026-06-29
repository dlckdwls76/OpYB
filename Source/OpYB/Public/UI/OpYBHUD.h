#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "OpYBHUD.generated.h"

class UOpYBAimCursorWidget;
class UOpYBPlayerCountWidget;
class UOpYBDeathScreenWidget;

/**
 * UI Manager HUD Class
 */
UCLASS()
class OPYB_API AOpYBHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	/** Shows the death screen */
	void ShowDeathScreen();

	/** Hides the death screen */
	void HideDeathScreen();

	/** Plays shoot animation on the aim cursor */
	void PlayShootAnimation();

	/** Updates the death screen time */
	void UpdateDeathScreenTime(float TimeLeft);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOpYBAimCursorWidget> AimCursorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOpYBPlayerCountWidget> PlayerCountClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UOpYBDeathScreenWidget> DeathScreenClass;

	UPROPERTY()
	TObjectPtr<UOpYBAimCursorWidget> AimCursorInstance;

	UPROPERTY()
	TObjectPtr<UOpYBPlayerCountWidget> PlayerCountInstance;

	UPROPERTY()
	TObjectPtr<UOpYBDeathScreenWidget> DeathScreenInstance;
};
