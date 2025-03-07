// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckPreviousAndCurrentState.generated.h"

/**
 * @brief 이전 상태와 현재 상태를 각각 다른 GameplayTag로 검사하는 데코레이터
 * 
 * - bCheckPreviousState == true 이면, PreviousState가 CheckPreviousState와 일치해야 함
 * - bCheckCurrentState == true 이면, CurrentState가 CheckCurrentState와 일치해야 함
 * - 둘 다 사용하면 AND 조건 (둘 다 만족해야 true)
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CheckPreviousAndCurrentState : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckPreviousAndCurrentState();

protected:
	/** 실제 조건 계산 로직 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

public:
	/** 이전 상태를 검사할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|State")
	bool bCheckPreviousState = false;

	/** 이전 상태로 체크할 GameplayTag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|State", meta=(EditCondition="bCheckPreviousState"))
	EAIState CheckPreviousState;

	/** 현재 상태를 검사할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|State")
	bool bCheckCurrentState = false;

	/** 현재 상태로 체크할 GameplayTag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|State", meta=(EditCondition="bCheckCurrentState"))
	EAIState CheckCurrentState;
};
