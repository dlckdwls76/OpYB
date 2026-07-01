// 에픽게임즈 저작권 소유.


#include "StrategyPawn.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"

AStrategyPawn::AStrategyPawn()
{
 	PrimaryActorTick.bCanEverTick = true;

	// 루트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 카메라 생성
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);

	// 무브먼트 컴포넌트 생성
	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Floating Pawn Movement"));

	// 카메라 설정
	Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
	Camera->OrthoWidth = 1500.0f;
	Camera->AutoPlaneShift = 1.0f;
	Camera->bUpdateOrthoPlanes = false;

	// 무브먼트 컴포넌트 설정
	FloatingPawnMovement->bConstrainToPlane = true;
	FloatingPawnMovement->SetPlaneConstraintNormal(FVector::UpVector);
	FloatingPawnMovement->SetPlaneConstraintOrigin(FVector::UpVector * 1500.0f);
}

void AStrategyPawn::SetZoomModifier(float Value)
{
	// 카메라의 오쏘 너비 설정
	Camera->SetOrthoWidth(Value);
}
