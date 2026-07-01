// 에픽게임즈 저작권 소유.


#include "TwinStickGameMode.h"
#include "TwinStickUI.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void ATwinStickGameMode::BeginPlay()
{
	// 아직 생성되지 않은 경우 UI 위젯 생성
	CreateUI();
}

void ATwinStickGameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	// 콤보 타이머 초기화
	GetWorld()->GetTimerManager().ClearTimer(ComboTimer);
}

void ATwinStickGameMode::ItemUsed(int32 Value)
{
	// UI 위젯을 사용할 수 있는지 확인
	if (!UIWidget)
	{
		CreateUI();
	}

	// UI 업데이트
	UIWidget->UpdateItems(Value);
}

void ATwinStickGameMode::ScoreUpdate(int32 Value)
{
	// 기본 점수에 콤보 배율을 곱하고 점수에 추가
	Score += Value * Combo;

	// UI 업데이트
	UIWidget->UpdateScore(Score);

	// 콤보 배율 업데이트
	ComboUpdate();
}

void ATwinStickGameMode::CreateUI()
{
	// UI를 여러 번 생성하지 않도록 방지
	if(UIWidget)
		return;

	// UI 위젯을 생성하고 뷰포트에 추가
	UIWidget = CreateWidget<UTwinStickUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), UIWidgetClass);
	UIWidget->AddToViewport(0);
}

void ATwinStickGameMode::ComboUpdate()
{
	// 반환
	if (Combo > ComboCap)
	{
		return;
	}

	// 콤보 증가량 업데이트
	++ComboIncrement;

	// 배율을 증가시킬 시간입니까?
	if (ComboIncrement > ComboIncrementMax)
	{
		// 콤보 증가량 초기화
		ComboIncrement = 0;

		// 콤보 배율 증가
		++Combo;

		// UI 업데이트
		UIWidget->UpdateCombo(Combo);

	}

	// 쿨타임 타이머 초기화
	ResetComboCooldown();
}

void ATwinStickGameMode::ResetComboCooldown()
{
	// 콤보 쿨타임 타이머 초기화
	GetWorld()->GetTimerManager().SetTimer(ComboTimer, this, &ATwinStickGameMode::ResetCombo, ComboCooldown, false);
}

void ATwinStickGameMode::ResetCombo()
{
	// 콤보 배율이 최소값 이상입니까?
	if (Combo > 1)
	{
		// 콤보 증가량 초기화
		ComboIncrement = 0;

		// 배율 감소 틱
		--Combo;

		// UI 업데이트
		UIWidget->UpdateCombo(Combo);

		// 쿨타임 타이머 초기화
		ResetComboCooldown();
	}
}

bool ATwinStickGameMode::CanSpawnNPCs()
{
	// NPC 카운터가 상한 미만입니까?
	return NPCCount < NPCCap;
}

void ATwinStickGameMode::IncreaseNPCs()
{
	// NPC 카운터 증가
	++NPCCount;
}

void ATwinStickGameMode::DecreaseNPCs()
{
	// NPC 카운터 감소
	--NPCCount;
}
