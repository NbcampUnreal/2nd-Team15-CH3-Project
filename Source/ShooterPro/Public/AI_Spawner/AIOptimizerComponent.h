// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "AIOptimizerComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTERPRO_API UAIOptimizerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAIOptimizerComponent();

	virtual void BeginPlay() override;

	/**
	* Bitmasks can be used to disable features for optimization.
	* ��Ʈ����ũ�� Ȱ���� ����ȭ�� ���� ��� ��Ȱ��ȭ�� �����մϴ�.
	*/
	UFUNCTION(BlueprintCallable)
	void OptimizerSetting(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ShooterPro.EAIOptimizerFlags"))int32 OptimizerEnable);
	//ACharacter* Character, AAIController* AIC, 
protected:
	// A short range layer that recognizes players. Not disabled.
	// �÷��̾ �ν��ϴ� ª�� ������ ���̾�. ��� ��Ȱ��ȭ ����.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Radius")
	float LayerShort;
	// Mid-range layer that recognizes the player. Shadows, Actor ticks disabled.
	// �÷��̾ �ν��ϴ� �߰� ������ ���̾�. �׸���, ���� ƽ ��Ȱ��ȭ.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Radius")
	float LayerMiddle;
	// Long range layer that recognizes the player. Disable all features except animation, collision.
	// �÷��̾ �ν��ϴ� �� ������ ���̾�. �ִϸ��̼�, �ݸ��� ������ ��� ��� ��Ȱ��ȭ.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Radius")
	float LayerLong;

protected:
	// Stop the behavior tree
	// �����̺�� Ʈ�� ����
	void SetAILogicEnabled(ACharacter* Character, AAIController* AIC, bool bEnable);
	// Stop the character movement
	// ĳ���� �����Ʈ ����
	void SetCharacterMovementEnabled(ACharacter* Character, bool bEnable);

	// Checking the distance to the player and determining which layers to run
	// �÷��̾���� �Ÿ��� Ȯ���ϰ� � ���̾ �����ų�� ���� 
	void LayerCheckLoop();
	// Calculating distance to players
	// �÷��̾���� �Ÿ� ���
	int32 DistanceLayer();

	FTimerHandle LayerCheckLoopTimer;
};
