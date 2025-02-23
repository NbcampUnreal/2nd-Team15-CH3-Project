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
	APatrolPath();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UFUNCTION(BlueprintCallable, Category="Patrol|Path")
	void IncrementPatrolRoute();

	UFUNCTION(BlueprintCallable, Category="Patrol|Path")
	FVector GetSplinePointAsWorldPosition();

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Patrol Path")
	USceneComponent* SceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Patrol Path")
	TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Patrol Path")
	TObjectPtr<UBillboardComponent> BillboardComponent;

public:
	UPROPERTY(BlueprintReadWrite, Category="Patrol|Path")
	int32 PatrolIndex;

	UPROPERTY(BlueprintReadWrite, Category="Patrol|Path")
	bool bPatrolReverseDirection;
};
