#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "QuickBarComponent.generated.h"

class UInventoryItemDefinition;
class UEquipmentDefinition;
class UInventoryItemInstance;
class UEquipmentManagerComponent;
class UEquipmentInstance;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuickBarSlotChanged, int32, OldSlotIndex, int32, NewSlotIndex);

USTRUCT(BlueprintType)
struct FQuickBarSlotData
{
	GENERATED_BODY()

public:
	/** 0-based 슬롯 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	int32 SlotIndex = -1;

	/** 누가(어떤 액터가) 이 슬롯 데이터를 소유하는지 */
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;
	
	/** 이 슬롯이 보유한 인벤토리 아이템 인스턴스 */
	UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	TObjectPtr<UInventoryItemInstance> InventoryItemInstance = nullptr;

	/** 인벤토리 아이템 정의 */
	UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	TSubclassOf<UInventoryItemDefinition> InventoryDef;

	/** 장비 인스턴스 (해당 슬롯이 실제로 Equip되어 있을 경우) */
	UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	TObjectPtr<UEquipmentInstance> EquipmentInstance = nullptr;

	/** 장비 정의 (Equippable 아이템의 경우) */
	UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	TSubclassOf<UEquipmentDefinition> EquipmentDef;


	//Test 및 더미코드 필요하면 사용
	// /** 전체 슬롯(인벤토리 아이템) 목록 (필요 시 사용) */
	// UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	// TArray<TObjectPtr<UInventoryItemInstance>> Slots;
	//
	// /** 전체 중에서 현재 활성화된 슬롯 인덱스 */
	// UPROPERTY(BlueprintReadOnly, Category="QuickBar")
	// int32 ActiveIndex = -1;
};


UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UQuickBarComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	void CheckSlots();

	UFUNCTION(BlueprintCallable, Category="Lyra")
	void CycleActiveSlotForward();

	UFUNCTION(BlueprintCallable, Category="Lyra")
	void CycleActiveSlotBackward();

	UFUNCTION(BlueprintCallable, Category="Lyra")
	void SetActiveSlotIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category="Lyra")
	void ChangeQuickBarSlot(int32 NewIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	TArray<UInventoryItemInstance*> GetSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	UInventoryItemInstance* GetActiveSlotItem() const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false)
	int32 GetNextFreeItemSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddItemToSlot(int32 SlotIndex, UInventoryItemInstance* Item);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	/**
	 * 단일 슬롯에 대한 상세 정보(예: UI가 필요로 할 때) 
	 * 내부적으로 EquipItem() 같은 부작용은 일으키지 말고, 단순히 현재 상태를 조회해서 반환
	 */
	UFUNCTION(BlueprintCallable)
	FQuickBarSlotData MakeSlotData(int32 SlotIndex) const;

private:
	void EquipItemInSlot();

	void UnequipItemInSlot();

	UEquipmentManagerComponent* FindEquipmentManager() const;

protected:
	UPROPERTY()
	int32 NumSlots = 3;

private:
	TArray<TObjectPtr<UInventoryItemInstance>> Slots;

	int32 ActiveSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<UEquipmentInstance> EquippedItem;

public:
	UPROPERTY(BlueprintAssignable, Category="QuickBar")
	FOnQuickBarSlotChanged OnSlotChanged;
};
