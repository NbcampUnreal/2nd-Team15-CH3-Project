#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetRandomIndexOnEnter.generated.h"

/** 
 * 브랜치(Selector/Composite)가 활성화될 때마다(진입 시) 블랙보드에 무작위 인덱스를 설정하는 서비스.
 * Tick은 쓰지 않고 OnBecomeRelevant/OnCeaseRelevant만 사용.
 */
USTRUCT()
struct FBTServiceRandomIndexMemory
{
	GENERATED_BODY()

	/** 이미 설정한 적이 있는지 여부 */
	bool bHasSetIndex = false;
};

/**
 * 자식 노드 중 하나를 고르기 위해 
 * 블랙보드의 int 키에 무작위 인덱스를 설정하는 서비스
 */
UCLASS()
class SHOOTERPRO_API UBTService_SetRandomIndexOnEnter : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetRandomIndexOnEnter(const FObjectInitializer& ObjectInitializer);

	/** 무작위 인덱스 범위 설정: [MinIndex..MaxIndex] */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	int32 MinIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	int32 MaxIndex = 3;

	/** 
	 * 브랜치가 비활성화(꺼질)될 때, bHasSetIndex를 다시 false로 되돌릴 지 여부.
	 * true로 설정하면 => 다음에 다시 OnBecomeRelevant가 호출될 때 새 무작위값을 뽑게 됨.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	bool bResetWhenCeaseRelevant = true;

protected:
	/** 메모리 구조체 크기 */
	virtual uint16 GetInstanceMemorySize() const override;

	/** 브랜치가 활성화될 때(진입 시) */
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 브랜치가 비활성화될 때(종료 시) */
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 실제 무작위 인덱스 설정 로직 */
	void SetRandomIndex(UBehaviorTreeComponent& OwnerComp, FBTServiceRandomIndexMemory* MyMemory);

};
