// 에픽게임즈 저작권 소유.


#include "StrategyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "StrategyPawn.h"
#include "Camera/CameraComponent.h"
#include "InputActionValue.h"
#include "StrategyHUD.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/GameplayStatics.h"
#include "StrategyUnit.h"
#include "NavigationSystem.h"
#include "Engine/OverlapResult.h"

AStrategyPlayerController::AStrategyPlayerController()
{
	// 마우스 커서는 항상 표시되어야 합니다.
	bShowMouseCursor = true;
}

void AStrategyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 로컬 플레이어 컨트롤러에서만 입력 설정
	if (IsLocalPlayerController())
	{
		// 입력 매핑 컨텍스트 추가
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			// 입력 모드에 기반한 컨텍스트 선택
			UInputMappingContext* ChosenContext = nullptr;

			switch (InputMode)
			{
			case SIM_Mouse:
				ChosenContext = MouseMappingContext;
				break;
			case SIM_Touch:
				ChosenContext = TouchMappingContext;
				break;
			}

			Subsystem->AddMappingContext(ChosenContext, 0);
		}

		// 입력 매핑 바인딩
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// 카메라
			EnhancedInputComponent->BindAction(MoveCameraAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::MoveCamera);
			EnhancedInputComponent->BindAction(ZoomCameraAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::ZoomCamera);
			EnhancedInputComponent->BindAction(ResetCameraAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::ResetCamera);

			// 마우스 상호작용
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::SelectHoldStarted);
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::SelectHoldTriggered);
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectHoldCompleted);
			EnhancedInputComponent->BindAction(SelectHoldAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::SelectHoldCompleted);

			EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectClick);

			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::SelectionModifier);
			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::SelectionModifier);
			EnhancedInputComponent->BindAction(SelectionModifierAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::SelectionModifier);

			EnhancedInputComponent->BindAction(InteractHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::InteractHoldStarted);
			EnhancedInputComponent->BindAction(InteractHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::InteractHoldTriggered);

			EnhancedInputComponent->BindAction(InteractClickAction, ETriggerEvent::Started, this, &AStrategyPlayerController::InteractClickStarted);
			EnhancedInputComponent->BindAction(InteractClickAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::InteractClickCompleted);

			// 터치 상호작용
			EnhancedInputComponent->BindAction(TouchPrimaryHoldAction, ETriggerEvent::Started, this, &AStrategyPlayerController::TouchPrimaryHoldStarted);
			EnhancedInputComponent->BindAction(TouchPrimaryHoldAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::TouchPrimaryHoldTriggered);
			EnhancedInputComponent->BindAction(TouchPrimaryHoldAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::TouchPrimaryHoldCompleted);

			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Started, this, &AStrategyPlayerController::TouchSecondaryStarted);
			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Triggered, this, &AStrategyPlayerController::TouchSecondaryTriggered);
			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Completed, this, &AStrategyPlayerController::TouchSecondaryCompleted);
			EnhancedInputComponent->BindAction(TouchSecondaryAction, ETriggerEvent::Canceled, this, &AStrategyPlayerController::TouchSecondaryCompleted);

		}
	}
}

void AStrategyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 올바른 폰 유형인지 확인
	ControlledPawn = Cast<AStrategyPawn>(InPawn);
	check(ControlledPawn);

	// 폰의 카메라에서 확대/축소 레벨 설정
	DefaultZoom = CameraZoom = ControlledPawn->GetCamera()->OrthoWidth;

	// HUD 포인터 캐스팅
	StrategyHUD = Cast<AStrategyHUD>(GetHUD());
	check(StrategyHUD);
}

void AStrategyPlayerController::DragSelectUnits(const TArray<AStrategyUnit*>& Units)
{
	// 목록에 유닛이 있습니까?
	if (Units.Num() > 0)
	{
		// 이전 유닛들이 모두 선택 해제되었는지 확인
		DoDeselectAllCommand();

		// 각각의 새 유닛 선택
		for (AStrategyUnit* CurrentUnit : Units)
		{
			// 선택 목록에 유닛 추가
			ControlledUnits.Add(CurrentUnit);

			// 유닛 선택
			CurrentUnit->UnitSelected();
		}

	}
	else
	{

		// 박스 내에 아무것도 없으므로 현재 선택된 모든 유닛 선택 해제
		if (ControlledUnits.Num() > 0)
		{
			DoDeselectAllCommand();
		}

	}
}

const TArray<AStrategyUnit*>& AStrategyPlayerController::GetSelectedUnits()
{
	return ControlledUnits;
}

void AStrategyPlayerController::MoveCamera(const FInputActionValue& Value)
{
	FVector2D InputVector = Value.Get<FVector2D>();

	// 앞으로 이동 입력 컴포넌트 벡터 가져오기
	FRotator ForwardRot = GetControlRotation();
	ForwardRot.Pitch = 0.0f;

	// 오른쪽 이동 입력 컴포넌트 벡터 가져오기
	FRotator RightRot = GetControlRotation();
	ForwardRot.Pitch = 0.0f;
	ForwardRot.Roll = 0.0f;

	// 앞으로 이동 입력 추가
	ControlledPawn->AddMovementInput(ForwardRot.RotateVector(FVector::ForwardVector), InputVector.X + InputVector.Y);

	// 오른쪽 이동 입력 추가
	ControlledPawn->AddMovementInput(RightRot.RotateVector(FVector::RightVector), InputVector.X - InputVector.Y);

}

void AStrategyPlayerController::ZoomCamera(const FInputActionValue& Value)
{
	// 입력을 스케일링하고 현재 확대/축소 레벨에서 빼기
	float ZoomLevel = CameraZoom - (Value.Get<float>() * ZoomScaling);

	// 최소/최대 확대/축소 레벨로 고정
	CameraZoom = FMath::Clamp(ZoomLevel, MinZoomLevel, MaxZoomLevel);

	// 폰의 카메라 업데이트
	ControlledPawn->SetZoomModifier(CameraZoom);

}

void AStrategyPlayerController::ResetCamera(const FInputActionValue& Value)
{
	// 확대/축소 레벨을 초기값으로 재설정
	CameraZoom = DefaultZoom;

	// 폰의 카메라 업데이트
	ControlledPawn->SetZoomModifier(DefaultZoom);

}

void AStrategyPlayerController::SelectHoldStarted(const FInputActionValue& Value)
{
	// 선택 시작 위치 저장
	StartingSelectionPosition = GetMouseLocation();

}

void AStrategyPlayerController::SelectHoldTriggered(const FInputActionValue& Value)
{

	// 현재 마우스 위치 가져오기
	FVector2D SelectionPosition = GetMouseLocation();

	// 선택 박스의 크기 계산
	FVector2D SelectionSize = SelectionPosition - StartingSelectionPosition;

	// HUD의 선택 박스 업데이트
	if (StrategyHUD)
	{
		StrategyHUD->DragSelectUpdate(StartingSelectionPosition, SelectionSize, SelectionPosition, true);
	}
	
}

void AStrategyPlayerController::SelectHoldCompleted(const FInputActionValue& Value)
{
	// HUD의 드래그 박스 초기화
	if (StrategyHUD)
	{
		StrategyHUD->DragSelectUpdate(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, false);
	}
}

void AStrategyPlayerController::SelectClick(const FInputActionValue& Value)
{

	if (GetLocationUnderCursor(CachedSelection))
	{
		DoSelectionCommand();
	}
}

void AStrategyPlayerController::SelectionModifier(const FInputActionValue& Value)
{

	// 선택 수정자 플래그 업데이트
	bSelectionModifier = Value.Get<bool>();
}

void AStrategyPlayerController::InteractHoldStarted(const FInputActionValue& Value)
{

	// 상호작용 시작 위치 저장
	StartingInteractionPosition = GetMouseLocation();
}

void AStrategyPlayerController::InteractHoldTriggered(const FInputActionValue& Value)
{

	// 드래그 스크롤 실행 
	DoDragScrollCommand();
}

void AStrategyPlayerController::InteractClickStarted(const FInputActionValue& Value)
{

	// 상호작용 플래그 초기화
	ResetInteraction();
}

void AStrategyPlayerController::InteractClickCompleted(const FInputActionValue& Value)
{

	// 제어 목록에 유닛이 있고 커서 아래에 유효한 상호작용 위치가 있습니까?
	if (ControlledUnits.Num() > 0 && GetLocationUnderCursor(CachedInteraction))
	{
		// 더블 탭 모두 선택이 활성화되었습니까?
		if (bDoubleTapActive)
		{
			// 더블 탭 모두 선택 해제
			bDoubleTapActive = false;

		}
		else
		{

			// 선택한 유닛을 대상 위치로 이동
			DoMoveUnitsCommand();

		}
	}
}

void AStrategyPlayerController::TouchPrimaryHoldStarted(const FInputActionValue& Value)
{
	// 탭 누른 시간 저장
	LastTapPressTime = GetWorld()->GetRealTimeSeconds();

	// 상호작용 시작 위치 저장
	StartingInteractionPosition = Value.Get<FVector2D>();
}

void AStrategyPlayerController::TouchPrimaryHoldTriggered(const FInputActionValue& Value)
{
	// 이 터치가 탭보다 깁니까?
	if ((GetWorld()->GetRealTimeSeconds() - LastTapPressTime) > TouchTapMaxAllowedTime)
	{
		// 박스 선택 중이 아니면 드래그 스크롤 실행
		if (!bSelectionModifier)
		{
			DoDragScrollCommand();
		}
	}
}

void AStrategyPlayerController::TouchPrimaryHoldCompleted(const FInputActionValue& Value)
{
	// 탭인지 더블 탭인지 확인합니다.
	// 터치 입력에서는 EnhancedInput 탭 트리거가 다르게 작동하므로 수동으로 이 작업을 수행해야 합니다.
	bool bTapped = false;
	bool bDoubleTapped = false;

	CheckTouchTap(bTapped, bDoubleTapped);

	// 더블 탭입니까?
	if (bTapped)
	{
		if (bDoubleTapped)
		{
			// 박스 선택을 하고 있지 않은지 확인
			if (!bSelectionModifier)
			{
				// 더블 탭 토글에 따라 모든 유닛 선택 또는 선택 해제
				if (bDoubleTapActive)
				{
					DoDeselectAllCommand();
				}
				else
				{
					DoSelectAllOnScreenCommand();
				}

				// 더블 탭 플래그 토글
				bDoubleTapActive = !bDoubleTapActive;
			}
		}

	// 더블 탭 없음, 이 터치 입력을 정상적으로 처리
	}
	else
	{

		// 이미 박스 선택 중이 아니거나 방금 전까지 박스 선택 중이 아니었는지 확인
		if (!(bSelectionModifier || (GetWorld()->GetRealTimeSeconds() - LastBoxSelectTime) < TouchTapMaxAllowedTime))
		{
			// 터치 위치를 투영하고 선택 지점을 캐시
			CachedInteraction = CachedSelection = ProjectTouchPointToWorldSpace();

			// 캐시된 위치로 선택 작업 실행
			DoSelectionCommand();
		}

	}

}

void AStrategyPlayerController::TouchSecondaryStarted(const FInputActionValue& Value)
{

	// 선택 수정자 플래그 올리기
	bSelectionModifier = true;

	// 두 번째 손가락의 시작 위치 저장
	StartingSecondFingerPosition = Value.Get<FVector2D>();
}

void AStrategyPlayerController::TouchSecondaryTriggered(const FInputActionValue& Value)
{
	// 두 번째 손가락의 현재 위치 업데이트
	CurrentSecondFingerPosition = Value.Get<FVector2D>();

	// 박스 선택 중이고 터치스크린에서 손가락이 충분히 움직였습니까?
	if (bSelectionModifier && !StartingSecondFingerPosition.Equals(CurrentSecondFingerPosition, 10.0f))
	{
		// 현재 상호작용 위치 업데이트
		CurrentInteractionPosition = CurrentSecondFingerPosition;

		// HUD의 선택 박스 업데이트
		if (StrategyHUD)
		{
			const FVector2D DragSize = CurrentSecondFingerPosition - StartingSecondFingerPosition;

			StrategyHUD->DragSelectUpdate(StartingInteractionPosition, DragSize, CurrentSecondFingerPosition, true);
		}
	}
}

void AStrategyPlayerController::TouchSecondaryCompleted(const FInputActionValue& Value)
{

	// 선택 수정자 플래그 해제
	bSelectionModifier = false;

	// 마지막 박스 선택 시간 저장
	LastBoxSelectTime = GetWorld()->GetRealTimeSeconds();

	// HUD에서 선택 박스 숨기기
	if (StrategyHUD)
	{
		StrategyHUD->DragSelectUpdate(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, false);
	}
}

void AStrategyPlayerController::DoSelectionCommand()
{

	// 선택할 액터를 찾기 위해 구체 스윕 수행
	FHitResult OutHit;

	const FVector Start = CachedSelection;
	const FVector End = Start + FVector::UpVector * 350.0f;

	FCollisionShape InteractionSphere;
	InteractionSphere.SetSphere(InteractionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetPawn());
	QueryParams.bTraceComplex = true;

	GetWorld()->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectParams, InteractionSphere, QueryParams);

	// 마우스를 사용 중이고 선택 수정자 키를 누르고 있지 않으면 먼저 모든 유닛 선택 해제
	if (InputMode == SIM_Mouse && !bSelectionModifier)
	{

		DoDeselectAllCommand();
	}

	// 유닛에 맞았습니까?
	if (OutHit.bBlockingHit)
	{

		// 대상 유닛 업데이트
		TargetUnit = Cast<AStrategyUnit>(OutHit.GetActor());

		if (TargetUnit)
		{

			// 유닛이 이미 제어 목록에 있습니까?
			if (ControlledUnits.Contains(TargetUnit))
			{

				// 제어 목록에서 유닛 제거
				ControlledUnits.Remove(TargetUnit);

				// 유닛에게 선택 해제되었음을 알림
				TargetUnit->UnitDeselected();

			}
			else
			{

				// 제어 목록에 유닛 추가
				ControlledUnits.Add(TargetUnit);

				// 유닛에게 선택되었음을 알림
				TargetUnit->UnitSelected();

			}
		}

	}
	else
	{

		// 터치 입력을 사용 중입니까?
		if (InputMode == SIM_Touch)
		{
			// 선택한 모든 유닛을 대상 위치로 이동
			DoMoveUnitsCommand();
		}

	}
}

void AStrategyPlayerController::DoSelectAllOnScreenCommand()
{

	// 현재 화면에 있는 모든 NPC 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStrategyUnit::StaticClass(), FoundActors);

	// 찾은 각 액터 처리
	for (AActor* CurrentActor : FoundActors)
	{
		// 유닛 클래스로 캐스팅
		if (AStrategyUnit* CurrentUnit = Cast<AStrategyUnit>(CurrentActor))
		{
			// 액터가 최근에 렌더링되었습니까?
			if (CurrentActor->WasRecentlyRendered(0.2f))
			{

				// 액터가 제어 유닛 목록에 없습니까?
				if (!ControlledUnits.Contains(CurrentUnit))
				{
					// 제어되는 유닛 목록에 추가
					ControlledUnits.Add(CurrentUnit);

					// 선택되었음을 알림
					CurrentUnit->UnitSelected();
				}
			}
		}		
	}

}

void AStrategyPlayerController::DoDeselectAllCommand()
{

	// 제어되는 각 유닛에게 선택 해제되었음을 알림
	for (AStrategyUnit* CurrentUnit : ControlledUnits)
	{
		// 유닛이 파괴되지 않았는지 확인
		if (IsValid(CurrentUnit))
		{

			CurrentUnit->UnitDeselected();
		}
	}

	// 제어되는 유닛 목록 지우기
	ControlledUnits.Empty();
}

void AStrategyPlayerController::DoDragScrollCommand()
{

	// 입력 모드에 기반한 커서 위치 선택
	FVector2D WorkingPosition;
	
	if (InputMode == EStrategyInputMode::SIM_Mouse)
	{

		// 마우스 위치 읽기
		bool bResult = GetMousePosition(WorkingPosition.X, WorkingPosition.Y);

	}
	else
	{

		// 터치 1 위치 읽기
		bool bPressed;
		GetInputTouchState(ETouchIndex::Touch1, WorkingPosition.X, WorkingPosition.Y, bPressed);

	}

	// 상호작용 시작 위치와 현재 좌표의 차이 찾기
	const FVector2D InteractionDelta = StartingInteractionPosition - WorkingPosition;

	const FRotator CameraRot(0.0f, -45.0f, 0.0f);

	// 상호작용 델타 회전 및 스케일 조정
	const FVector ScrollDelta = CameraRot.RotateVector(FVector(InteractionDelta.X, InteractionDelta.Y, 0.0f)) * DragMultiplier;

	// 제어되는 폰에 월드 오프셋 적용
	ControlledPawn->AddActorWorldOffset(ScrollDelta);
}

void AStrategyPlayerController::DoMoveUnitsCommand()
{

	// 이동 목표 설정
	FVector CurrentMoveGoal;

	if (InputMode == EStrategyInputMode::SIM_Mouse)
	{

		// 캐시된 상호작용 지점을 이동 목표로 설정
		CurrentMoveGoal = CachedInteraction;

	}
	else
	{

		// 캐시된 선택 영역을 이동 목표로 설정
		CurrentMoveGoal = CachedSelection;

	}

	// 이동 목표에 가장 가까운 선택된 유닛 가져오기. 이 유닛이 리드 유닛이 됩니다.
	AStrategyUnit* Closest = GetClosestSelectedUnitToLocation(CurrentMoveGoal);

	// 이동 요청 중 하나라도 실패하면 true로 설정됩니다.
	bool bInteractionFailed = false;

	// 제어 목록의 각 유닛 처리
	for (AStrategyUnit* CurrentUnit : ControlledUnits)
	{
		if (IsValid(CurrentUnit))
		{

			// 유닛 정지
			CurrentUnit->StopMoving();

			// 리드 유닛을 목표 지점으로 이동하고, 나머지 유닛은 그 주변의 무작위 항행 가능 지점으로 이동
			FVector MoveGoal = CurrentMoveGoal;

			if (CurrentUnit != Closest)
			{

				UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(GetWorld(), CurrentMoveGoal, MoveGoal, InteractionRadius * 0.66f);
			}

			// 유닛의 이동 완료 델리게이트 구독
			CurrentUnit->OnMoveCompleted.AddDynamic(this, &AStrategyPlayerController::OnMoveCompleted);

			// 목표 위치로의 이동 설정
			if (!CurrentUnit->MoveToLocation(MoveGoal, InteractionRadius * 0.66f))
			{
				// 이동 요청이 실패했으므로 플래그 설정
				bInteractionFailed = true;
			}
		}

	}

	// 이동 성공 여부에 따라 커서 피드백 재생
	BP_CursorFeedback(CachedInteraction, !bInteractionFailed);

}

void AStrategyPlayerController::OnMoveCompleted(AStrategyUnit* MovedUnit)
{
	// 유닛이 유효합니까?
	if (IsValid(MovedUnit))
	{
		// 델리게이트 구독 취소
		MovedUnit->OnMoveCompleted.RemoveDynamic(this, &AStrategyPlayerController::OnMoveCompleted);
		
		// 상호작용이 잠겨 있으면 건너뛰기
		if (!bAllowInteraction)
		{
			return;
		}

		// 초기화될 때까지 추가 상호작용 방지
		bAllowInteraction = false;

		// 유닛이 캐시된 상호작용 위치에 충분히 가깝습니까?
		if(FVector::Dist2D(CachedInteraction, MovedUnit->GetActorLocation()) < InteractionRadius)
		{

			// 인접한 상호작용 가능한 객체를 찾기 위해 오버랩 테스트 수행
			TArray<FOverlapResult> OutOverlaps;

			FCollisionShape CollisionSphere;
			CollisionSphere.SetSphere(InteractionRadius);

			FCollisionObjectQueryParams ObjectParams;
			ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
			
			FCollisionQueryParams QueryParams;

			QueryParams.AddIgnoredActor(MovedUnit);

			for(const AStrategyUnit* CurSelected : ControlledUnits)
			{
				QueryParams.AddIgnoredActor(CurSelected);
			}

			if (GetWorld()->OverlapMultiByObjectType(OutOverlaps, CachedInteraction, FQuat::Identity, ObjectParams, CollisionSphere, QueryParams))
			{
				for (const FOverlapResult& CurrentOverlap : OutOverlaps)
				{
					if (AStrategyUnit* CurrentUnit = Cast<AStrategyUnit>(CurrentOverlap.GetActor()))
					{
						CurrentUnit->Interact(MovedUnit);
					}
				}
			}
		}
	}
}

AStrategyUnit* AStrategyPlayerController::GetClosestSelectedUnitToLocation(FVector TargetLocation)
{
	// 가장 가까운 유닛과 거리
	AStrategyUnit* OutUnit = nullptr;
	float Closest = 0.0f;

	// 목록의 각 유닛 처리
	for (AStrategyUnit* CurrentUnit : ControlledUnits)
	{
		if (CurrentUnit != nullptr)
		{
			// 이미 유닛을 선택했습니까?
			if (OutUnit != nullptr)
			{
				// 대상 위치까지의 거리 제곱 계산
				float Dist = FVector::DistSquared2D(TargetLocation, CurrentUnit->GetActorLocation());

				// 이 유닛이 더 가깝습니까?
				if (Dist < Closest)
				{
					// 가장 가까운 유닛 및 거리 업데이트
					OutUnit = CurrentUnit;
					Closest = Dist;
				}

			}
			else
			{

				// 이전에 선택한 유닛이 없으므로 이 유닛 사용
				OutUnit = CurrentUnit;

				// 가장 가까운 거리 초기화
				Closest = FVector::DistSquared2D(TargetLocation, CurrentUnit->GetActorLocation());
			}
		}
		
	}

	// 반환 the selected unit
	return OutUnit;
}

FVector2D AStrategyPlayerController::GetMouseLocation()
{
	// 이 PC에서 마우스 위치 가져오기 시도
	float MouseX, MouseY;

	if (GetMousePosition(MouseX, MouseY))
	{
		return FVector2D(MouseX, MouseY);
	}

	// 반환 an invalid vector
	return FVector2D::ZeroVector;
}

bool AStrategyPlayerController::GetLocationUnderCursor(FVector& Location)
{
	// 커서 위치에서 가시성 채널 트레이스
	FHitResult OutHit;

	GetHitResultUnderCursorByChannel(SelectionTraceChannel, true, OutHit);

	// 블로킹 히트가 있으면 히트 위치 반환
	if (OutHit.bBlockingHit)
	{
		Location = OutHit.Location;
		return true;
	}

	return OutHit.bBlockingHit;
}

FVector AStrategyPlayerController::ProjectTouchPointToWorldSpace()
{
	// 첫 번째 손가락의 터치 좌표 가져오기
	float TouchX, TouchY = 0.0f;
	bool bPressed = false;

	GetInputTouchState(ETouchIndex::Touch1, TouchX, TouchY, bPressed);

	FVector WorldLocation = FVector::ZeroVector;
	FVector WorldDirection = FVector::ZeroVector;

	// 좌표를 월드 공간으로 디프로젝트
	if (DeprojectScreenPositionToWorld(TouchX, TouchY, WorldLocation, WorldDirection))
	{
		// 수평 평면과 교차하고 결과 지점 반환
		const FPlane IntersectPlane(FVector::ZeroVector, FVector::UpVector);

		return FMath::LinePlaneIntersection(WorldLocation, WorldLocation + (WorldDirection * 100000.0f), IntersectPlane);
	}

	// 디프로젝트 실패, 영벡터 반환
	return FVector::ZeroVector;
}

void AStrategyPlayerController::ResetInteraction()
{
	bAllowInteraction = true;
}

void AStrategyPlayerController::CheckTouchTap(bool& bTapped, bool& bDoubleTapped)
{
	// 현재 게임 시간 가져오기
	const float GameTime = GetWorld()->GetRealTimeSeconds();

	// 누른 후 최대 허용 시간 이전에 플레이어가 터치를 해제하면 탭입니다.
	bTapped = (GameTime - LastTapPressTime) < TouchTapMaxAllowedTime;

	if (bTapped)
	{
		// 마지막 해제 시간 전에 다른 탭이 발생했으면 더블 탭입니다.
		if ((GameTime - LastTapReleaseTime) < TouchDoubleTapMaxAllowedTime)
		{
			// 탭 카운터 증가
			++TapCount;

		}
		else
		{

			// 탭 카운터 초기화
			TapCount = 0;
		}

	}
	else
	{

		// 탭 카운터 초기화
		TapCount = 0;
	}

	// 탭 카운트가 0이 아니면 더블 탭입니다.
	bDoubleTapped = TapCount >= 1;

	// 탭 뗀 시간 저장
	LastTapReleaseTime = GameTime;
}
