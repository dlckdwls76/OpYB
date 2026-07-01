// 에픽게임즈 저작권 소유.


#include "TwinStickPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "TwinStickCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "OpYB.h"
#include "Widgets/Input/SVirtualJoystick.h"

void ATwinStickPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어 컨트롤러에서만 터치 컨트롤 스폰
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// 모바일 컨트롤 위젯 스폰
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// 플레이어 화면에 컨트롤 추가
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogOpYB, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void ATwinStickPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 로컬 플레이어 컨트롤러에 대해서만 IMC 추가
	if (IsLocalPlayerController())
	{
		// 입력 매핑 컨텍스트 추가
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// 모바일 터치 입력을 사용하지 않는 경우에만 이러한 IMC 추가
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void ATwinStickPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 폰의 파괴됨 델리게이트 구독
	InPawn->OnDestroyed.AddDynamic(this, &ATwinStickPlayerController::OnPawnDestroyed);
}

void ATwinStickPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// 플레이어 시작 지점 찾기
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// 플레이어 시작 지점에서 캐릭터 스폰
		const FTransform SpawnTransform = ActorList[0]->GetActorTransform();

		if (ATwinStickCharacter* RespawnedCharacter = GetWorld()->SpawnActor<ATwinStickCharacter>(CharacterClass, SpawnTransform))
		{
			// 캐릭터 빙의
			Possess(RespawnedCharacter);
		}
	}
}

bool ATwinStickPlayerController::ShouldUseTouchControls() const
{
	// 모바일 플랫폼입니까? 강제 터치를 해야 합니까?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
