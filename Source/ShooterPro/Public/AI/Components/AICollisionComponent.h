#pragma once

#include "CoreMinimal.h"
#include "AI/ShooterAITypes.h"
#include "Components/ActorComponent.h"
#include "AICollisionComponent.generated.h"


UCLASS(ClassGroup=("AI"), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UAICollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAICollisionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	void EnableCollision(ECollisionPart CollisionPart);
};
