#pragma once

#include "CoreMinimal.h"
#include "Character/Interfaces/Interface_CharacterStatus.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "ShooterAIBase.generated.h"


class UAICollisionComponent;
class UAIBehaviorsComponent;
class UBehaviorTree;


UCLASS()
class SHOOTERPRO_API AShooterAIBase : public ACharacter, public IInterface_CharacterStatus
{
	GENERATED_BODY()

public:
	AShooterAIBase();

	//~ Begin ACharacter Interface
protected:
	virtual void BeginPlay() override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Tick(float DeltaSeconds) override;

public:
	//~ Begin IInterface_CharacterStatus Interface
	virtual bool IsAlive_Implementation() override
	{
		return CurrentHealth > 0;
	}

	//~ Begin Rotate
public:
	UFUNCTION(BlueprintCallable, Category="AI Base|Rotate")
	void StartRotateToPlayer(float NewInterpSpeed);

	UFUNCTION(BlueprintCallable, Category="AI Base|Rotate")
	void StopRotateToPlayer();

protected:
	UFUNCTION()
	void OnRotateUpdateAlpha(float Alpha);


	//~ Properties
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="AI Base")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI Base")
	TObjectPtr<UAIBehaviorsComponent> AIBehaviorsComponent;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI Base")
	TObjectPtr<UAICollisionComponent> MeleeCollisionComponent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base|Rotate")
	float RotateInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base|Rotate")
	float RotateTimelineLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base|Rotate")
	UCurveFloat* RotateCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base", meta=(ExposeOnSpawn=true))
	float CurrentHealth;

protected:
	FTimeline RotateTimeline;
};
