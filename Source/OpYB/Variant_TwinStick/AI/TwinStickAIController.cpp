// 에픽게임즈 저작권 소유.


#include "TwinStickAIController.h"
#include "Components/StateTreeAIComponent.h"

ATwinStickAIController::ATwinStickAIController()
{
	// StateTree AI 컴포넌트 생성
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	check(StateTreeAI);

	// 폰에 빙의할 때 StateTree가 시작되는지 확인
	bStartAILogicOnPossess = true;

	// 빙의된 캐릭터에 연결되어 있는지 확인합니다.
	// EnvQueries가 올바르게 작동하려면 이것이 필요합니다.
	bAttachToPawn = true;
}