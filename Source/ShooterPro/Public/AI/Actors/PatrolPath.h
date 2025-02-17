
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPath.generated.h"

class USplineComponent;

UCLASS()
class SHOOTERPRO_API APatrolPath : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APatrolPath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Patrol Path")
	TObjectPtr<USplineComponent> SplineComponent;
};
