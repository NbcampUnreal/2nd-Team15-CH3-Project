// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_SetRandomIndexOnEnter.h"

#include "AI/EnemyAILog.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SetRandomIndexOnEnter::UBTService_SetRandomIndexOnEnter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Set Random Index On Enter");
	// Tick은 사용하지 않을 것이므로 Interval=0, bCallTickOnSearchStart=false
	Interval = 0.0f;
	bCallTickOnSearchStart = false;
	bNotifyTick=false;
	bNotifyBecomeRelevant=true;
	bNotifyCeaseRelevant=true;
}

uint16 UBTService_SetRandomIndexOnEnter::GetInstanceMemorySize() const
{
	return sizeof(FBTServiceRandomIndexMemory);
}

void UBTService_SetRandomIndexOnEnter::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	FBTServiceRandomIndexMemory* MyMemory = (FBTServiceRandomIndexMemory*)NodeMemory;
	if (!MyMemory->bHasSetIndex)
	{
		SetRandomIndex(OwnerComp, MyMemory);
	}
}

void UBTService_SetRandomIndexOnEnter::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	if (bResetWhenCeaseRelevant)
	{
		FBTServiceRandomIndexMemory* MyMemory = (FBTServiceRandomIndexMemory*)NodeMemory;
		MyMemory->bHasSetIndex = false;
	}
}

void UBTService_SetRandomIndexOnEnter::SetRandomIndex(UBehaviorTreeComponent& OwnerComp, FBTServiceRandomIndexMemory* MyMemory)
{
	// 안전 장치: Min이 Max보다 클 경우 처리
	if (MinIndex > MaxIndex)
	{
		AI_ENEMY_LOG_LOG("[SetRandomIndexOnEnter] MinIndex > MaxIndex!");
		return;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return;
	}

	const int32 RandValue = FMath::RandRange(MinIndex, MaxIndex);
	BBComp->SetValueAsInt(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_RandomInt(), RandValue);

	MyMemory->bHasSetIndex = true;

	// AI_ENEMY_LOG_LOG("[SetRandomIndexOnEnter] => %d (range %d~%d)", RandValue, MinIndex, MaxIndex);
}