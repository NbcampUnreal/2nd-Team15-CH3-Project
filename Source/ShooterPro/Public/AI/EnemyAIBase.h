#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyAIBase.generated.h"


class UAICollisionComponent;
class UAIBehaviorsComponent;
class UBehaviorTree;


UCLASS()
class SHOOTERPRO_API AEnemyAIBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyAIBase();

	//~ Begin ACharacter Interface
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

public:


	
	//~ Properties
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="AI Base")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI Base")
	TObjectPtr<UAIBehaviorsComponent> AIBehaviorsComponent;
};
