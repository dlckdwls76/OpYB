// 에픽게임즈 저작권 소유.


#include "TwinStickStateTreeUtility.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "TopDownTemplate"

EStateTreeRunStatus FStateTreeGetPlayerTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// 인스턴스 데이터 가져오기
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 첫 번째 로컬 플레이어가 빙의한 폰 가져오기
	InstanceData.TargetPlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(InstanceData.Character, 0));

	// 태스크를 계속 실행 상태로 유지
	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FStateTreeGetPlayerTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return LOCTEXT("StateTreeTaskGetPlayerDescription", "<b>Get Player</b>");
}
#endif // WITH_EDITOR