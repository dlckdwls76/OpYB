// 에픽게임즈 저작권 소유.


#include "StrategyUnit.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "Navigation/PathFollowingComponent.h"

AStrategyUnit::AStrategyUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	// 이 유닛이 이동 요청을 처리할 유효한 AI 컨트롤러를 가지고 있는지 확인
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 상호작용 범위 구체 생성
	InteractionRange = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Range"));
	InteractionRange->SetupAttachment(RootComponent);

	InteractionRange->SetSphereRadius(100.0f);
	InteractionRange->SetCollisionProfileName(FName("OverlapAllDynamic"));

	// 무브먼트 설정
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->MaxAcceleration = 1000.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.0f;
	GetCharacterMovement()->PerchRadiusThreshold = 20.0f;
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->AvoidanceConsiderationRadius = 150.0f;
	GetCharacterMovement()->AvoidanceWeight = 1.0f;
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->SetFixedBrakingDistance(200.0f);
	GetCharacterMovement()->SetFixedBrakingDistance(true);
}

void AStrategyUnit::NotifyControllerChanged()
{
	// AI 컨트롤러 레퍼런스 사본 유효성 검사 및 저장
	AIController = Cast<AAIController>(Controller);
	
	if (AIController)
	{
		// 경로 추적 컴포넌트의 이동 완료 핸들러 구독
		UPathFollowingComponent* PFComp = AIController->GetPathFollowingComponent();
		if (PFComp)
		{
			PFComp->OnRequestFinished.AddUObject(this, &AStrategyUnit::OnMoveFinished);
		}
	}
}

void AStrategyUnit::StopMoving()
{
	// 캐릭터 무브먼트 컴포넌트를 사용하여 이동 정지
	GetCharacterMovement()->StopMovementImmediately();
}

void AStrategyUnit::UnitSelected()
{
	// BP로 제어권 넘기기
	BP_UnitSelected();
}

void AStrategyUnit::UnitDeselected()
{
	// BP로 제어권 넘기기
	BP_UnitDeselected();
}

void AStrategyUnit::Interact(AStrategyUnit* Interactor)
{
	// 상호작용자가 유효한지 확인
	if (IsValid(Interactor))
	{
		// 상호작용 중인 액터 쪽으로 회전
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Interactor->GetActorLocation()));

		// 상호작용자에게 상호작용 동작을 재생하라는 신호 보내기
		Interactor->BP_InteractionBehavior(this);

		// 자체 상호작용 동작 재생
		BP_InteractionBehavior(Interactor);
	}
	
}

bool AStrategyUnit::MoveToLocation(const FVector& Location, float AcceptanceRadius)
{
	// 유효한 AI 컨트롤러가 있는지 확인
	if (AIController)
	{
		// AI 이동 요청 설정
		FAIMoveRequest MoveReq;

		MoveReq.SetGoalLocation(Location);
		MoveReq.SetAcceptanceRadius(AcceptanceRadius);
		MoveReq.SetAllowPartialPath(true);
		MoveReq.SetUsePathfinding(true);
		MoveReq.SetProjectGoalLocation(true);
		MoveReq.SetRequireNavigableEndLocation(true);
		MoveReq.SetNavigationFilter(AIController->GetDefaultNavigationFilterClass());
		MoveReq.SetCanStrafe(false);

		// AI 컨트롤러에 이동 요청
		FNavPathSharedPtr FollowedPath;
		const FPathFollowingRequestResult ResultData = AIController->MoveTo(MoveReq, &FollowedPath);
		
		// 이동 결과 확인
		switch (ResultData.Code)
		{
			// 실패함. false 반환
			case EPathFollowingRequestResult::Failed:

				return false;
				break;

			// 이미 목표지점입니다. true를 반환하고 이동 완료 델리게이트를 호출합니다.
			case EPathFollowingRequestResult::AlreadyAtGoal:

				OnMoveCompleted.Broadcast(this);
				return true;
				break;

			// 이동이 성공적으로 예약됨. true 반환
			case EPathFollowingRequestResult::RequestSuccessful:

				return true;
				break;
		}
	}

	// 이동을 완료할 수 없습니다.
	return false;
}

void AStrategyUnit::OnMoveFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	// 델리게이트 호출
	OnMoveCompleted.Broadcast(this);
}
