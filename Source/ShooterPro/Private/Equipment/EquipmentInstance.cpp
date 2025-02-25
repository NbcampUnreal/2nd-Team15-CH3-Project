// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/EquipmentInstance.h"

#include "Components/SphereComponent.h"
#include "Equipment/EquipmentDefinition.h"
#include "GameFramework/Character.h"

UEquipmentInstance::UEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), EquippedAnimMontage(nullptr), UnequippedAnimMontage(nullptr)
{

}

void UEquipmentInstance::OnInstanceCreated_Implementation()
{
	
}

UWorld* UEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetOwnerAsPawn())
	{
		return OwningPawn->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

APawn* UEquipmentInstance::GetOwnerAsPawn() const
{
	return Cast<APawn>(GetOuter());
}

APawn* UEquipmentInstance::GetOwnerAsType(TSubclassOf<APawn> Type) const
{
	APawn* Result = nullptr;
	if (UClass* PawnType = Type)
	{
		if (GetOuter()->IsA(PawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UEquipmentInstance::SpawnEquipmentActors(const TArray<FEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetOwnerAsPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();

		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);

			SpawnedActors.Add(NewActor);
		}
	}
}

void UEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor) Actor->Destroy();
	}
}

void UEquipmentInstance::OnEquipped()
{
	SetAnimMontage(EquippedAnimMontage);
	ACharacter* Character = GetOwnerAsTypeTemp<ACharacter>();
	if (!Character) return;
	Character->GetMesh()->LinkAnimClassLayers(EquippedLayer);
}

void UEquipmentInstance::OnUnequipped()
{
	SetAnimMontage(UnequippedAnimMontage);
}

void UEquipmentInstance::SetAnimMontage(UAnimMontage* Montage)
{
	if (!ensure(Montage)) return;
	
	ACharacter* Character = GetOwnerAsTypeTemp<ACharacter>();
	if (!ensure(Character)) return;

	auto Mesh = Character->GetMesh();
	if (!ensure(Mesh)) return;

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	
	if (!ensure(AnimInstance)) return;
	
	AnimInstance->Montage_Play(Montage);
}
