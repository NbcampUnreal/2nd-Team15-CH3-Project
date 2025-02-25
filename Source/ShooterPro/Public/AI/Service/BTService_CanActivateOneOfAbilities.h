// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CanActivateOneOfAbilities.generated.h"

/**
 * @brief 서비스:
 *   - AbilityTags 중 하나라도 활성화 가능한 Ability가 있는지 검사
 *   - 결과를 Boolean 형태로 블랙보드에 저장
 */
UCLASS()
class SHOOTERPRO_API UBTService_CanActivateOneOfAbilities : public UBTService
{
	GENERATED_BODY()

public:
	/** 생성자: 기본 노드 이름 설정 등 */
	UBTService_CanActivateOneOfAbilities();

protected:
	/** 정해진 간격(Interval)마다 TickNode가 호출됨 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	/**
	 * AbilityTags 컨테이너에 들어있는 태그들 중
	 * 하나라도 활성화 가능한 Ability가 있다면 true를 반환.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Ability")
	FGameplayTagContainer AbilityTags;

	/**
	 * TickNode에서 검사된 결과(활성화 가능한 Ability가 있으면 true/없으면 false)를
	 * 저장할 블랙보드 키.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Blackboard")
	struct FBlackboardKeySelector bHasActivableAbilityKey;
};
