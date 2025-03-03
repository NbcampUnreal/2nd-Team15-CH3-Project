// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ShooterProPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "ProGmaeplayTag.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Equipment/QuickBarComponent.h"
#include "Inventory/InventoryFragment_QuickBarIcon.h"
#include "Inventory/InventoryItemInstance.h"
#include "Inventory/InventoryManagerComponent.h"

AShooterProPlayerController::AShooterProPlayerController()
{
}

void AShooterProPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetLocalPlayer());
	
	if (ensure(LocalPlayer))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		
		if (ensure(Subsystem))
		{
			if (ensure(MappingContext)) Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
	
	// UQuickBarComponent* QuickBar = Cast<UQuickBarComponent>(GetPawn()->GetComponentByClass(UQuickBarComponent::StaticClass()));
	// if (QuickBar)
	// {
	// 	QuickBar->OnSlotChanged.AddDynamic(this, &AShooterProPlayerController::UpdateQuickBar);
	// }
}

void AShooterProPlayerController::UpdateQuickBar()
{
	// if (!UserWidget) return;
	//
	// UUserWidget* QuickBarUI = Cast<UUserWidget>(UserWidget->GetWidgetFromName("WBP_WeaponSlot"));

	// if (!QuickBarUI) return;
	//
	// UQuickBarComponent* QuickBar = Cast<UQuickBarComponent>(GetPawn()->GetComponentByClass(UQuickBarComponent::StaticClass()));
	// if (!QuickBar) return;
	//
	// TArray<UInventoryItemInstance*> ItemInstances = QuickBar->GetSlots();
	//
	// int32 ActiveIndex = QuickBar->GetActiveSlotIndex();
	//
	// Swap(ItemInstances[0], ItemInstances[ActiveIndex]);
	//
	// for (int32 SlotIndex = 0; SlotIndex < ItemInstances.Num(); ++SlotIndex)
	// {
	// 	UUserWidget* SlotWidget = Cast<UUserWidget>(QuickBarUI->GetWidgetFromName(FName(*FString::Printf(TEXT("WBP_WeaponSlot%d"), SlotIndex + 1))));
 //        
	// 	if (SlotWidget && ItemInstances.IsValidIndex(SlotIndex))
	// 	{
	// 		UpdateSlot(SlotWidget, ItemInstances[SlotIndex]);
	// 	}
	// }
}

void AShooterProPlayerController::UpdateSlot(UUserWidget* Slot, const UInventoryItemInstance* ItemInstance)
{
	UInventoryManagerComponent* InventoryManager = Cast<UInventoryManagerComponent>(GetPawn()->GetComponentByClass(UInventoryManagerComponent::StaticClass()));
	if (!InventoryManager) return;

	//UAbilitySystemComponent* AbilitySystem = Cast<UAbilitySystemComponent>(GetPawn()->GetComponentByClass(UGSCAbilitySystemComponent::StaticClass()));
	//if (AbilitySystem->HasMatchingGameplayTag(ProGameplayTags::Ability_Reload))
	//{
	//	//장전 애니메이션?
	//}
	const UInventoryFragment_QuickBarIcon* IconFragment = Cast<UInventoryFragment_QuickBarIcon>(ItemInstance->FindFragmentByClass(UInventoryFragment_QuickBarIcon::StaticClass()));
	if (!IconFragment) return;
		
	int32 CurrentAmmo = ItemInstance->GetStatTagStackCount(ProGameplayTags::Weapon_MagazineAmmo);
	int32 TotalAmmo = InventoryManager->GetItemStackCount(IconFragment->AmmoType);
	
	UTextBlock* CurBulletTextBlock = Cast<UTextBlock>(Slot->GetWidgetFromName("Text_CurrentBulletNum"));
	CurBulletTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentAmmo)));

	UTextBlock* TotalBulletTextBlock = Cast<UTextBlock>(Slot->GetWidgetFromName("Text_MaxBulletNum"));
	TotalBulletTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d"), TotalAmmo)));
	

	
	UImage* Img = Cast<UImage>(Slot->GetWidgetFromName("Image_Weapon"));
	Img->SetBrush(IconFragment->Brush);
}

void AShooterProPlayerController::QuickBarSlotChanged(int32 NewSlot)
{
	// UUserWidget* QuickBar = Cast<UUserWidget>(UserWidget->GetWidgetFromName("WBP_WeaponSlot"));
	//
	// if (!QuickBar) return;
	//
	// UUserWidget* Slot1 = Cast<UUserWidget>(UserWidget->GetWidgetFromName("WBP_WeaponSlot1"));
	
	
	// UFunction* PlayAnimFunc = nullptr;
	// switch (NewSlot)
	// {
	// case 0:
	// 	PlayAnimFunc = QuickBar->FindFunction(FName("SequenceEvent"));
	// 	break;
	// case 1:
	// 	PlayAnimFunc = QuickBar->FindFunction(FName("SequenceEvent"));
	// 	break;
	// case 2:
	// 	PlayAnimFunc = QuickBar->FindFunction(FName("SequenceEvent"));
	// 	break;
	// default: break;
	// }
	//
	// if (!PlayAnimFunc) return;
	//
	// QuickBar->ProcessEvent(PlayAnimFunc, nullptr);
	
}
