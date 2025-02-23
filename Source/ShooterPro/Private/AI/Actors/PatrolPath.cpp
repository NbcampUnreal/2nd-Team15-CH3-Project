#include "AI/Actors/PatrolPath.h"

#include "Components/BillboardComponent.h"
#include "Components/SplineComponent.h"

APatrolPath::APatrolPath()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(SceneComponent);
	SplineComponent->EditorSelectedSplineSegmentColor = FLinearColor(0.828f, 0.364f, 0.003f, 1.0f);

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillboardComponent"));
	BillboardComponent->SetupAttachment(SceneComponent);
	BillboardComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
}

void APatrolPath::BeginPlay()
{
	Super::BeginPlay();
}

void APatrolPath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APatrolPath::IncrementPatrolRoute()
{
	int32 StepDirection = (bPatrolReverseDirection) ? -1 : 1;
	int32 SplinePointNum = SplineComponent->GetNumberOfSplinePoints() - 1;

	PatrolIndex += StepDirection;

	if (PatrolIndex >= SplinePointNum)
		bPatrolReverseDirection = true;
	else if (PatrolIndex == 0)
		bPatrolReverseDirection = false;
}

FVector APatrolPath::GetSplinePointAsWorldPosition()
{
	return SplineComponent->GetLocationAtSplinePoint(PatrolIndex, ESplineCoordinateSpace::World);
}


void APatrolPath::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
