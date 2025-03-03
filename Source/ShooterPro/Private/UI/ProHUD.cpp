// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ProHUD.h"

#include "Blueprint/UserWidget.h"

AProHUD::AProHUD()
{
}

void AProHUD::BeginPlay()
{
	Super::BeginPlay();

	// CreateWidgets();
	// ShowWidgets();
}

// void AProHUD::ClearAllWidgets()
// {
// 	// if (UserWidget)
// 	// {
// 	// 	UserWidget->RemoveFromParent();
// 	// }
// }

// void AProHUD::CreateWidgets()
// {
// 	// if (UserWidgetClass)
// 	// {
// 	// 	UserWidget = CreateWidget<UUserWidget>(GetWorld(), UserWidgetClass);
// 	// }
// }

// void AProHUD::ShowWidgets()
// {
// 	// if (UserWidget) UserWidget->AddToViewport();
// }

// void AProHUD::UpdateQuickBar()
// {
// 	// if (!UserWidget) return;
// 	//
// 	// UWidget* QuickBar = UserWidget->GetWidgetFromName("WBP_WeaponSlot");
// 	//
// 	// if (!QuickBar) return;
// 	//
// 	// UWidget* Slot1 = UserWidget->GetWidgetFromName("WBP_WeaponSlot1");
// 	// UWidget* Slot2 = UserWidget->GetWidgetFromName("WBP_WeaponSlot2");
// 	// UWidget* Slot3 = UserWidget->GetWidgetFromName("WBP_WeaponSlot3");
// 	//
// 	// UFunction* PlayAnimFunc = Slot1->FindFunction(FName("ImageChange"));
// 	// if (PlayAnimFunc)
// 	// {
// 	// 	Slot1->ProcessEvent(PlayAnimFunc, nullptr);
// 	// }
// 	
// }

