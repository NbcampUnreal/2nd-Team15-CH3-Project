// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/BulletChargeItem.h"
#include "Item/ItemSpawnRow.h"
#include "Character/Player/ProPlayerCharacter.h"
#include "Inventory/InventoryManagerComponent.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Components/SphereComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ABulletChargeItem::ABulletChargeItem()
{
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	ActivationCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationCollision"));
	ActivationCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ActivationCollision->SetSphereRadius(100.0f);
	ActivationCollision->SetupAttachment(Scene);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->SetupAttachment(Scene);

	PickUpNiagaraSystem = nullptr;
	PickUpSound = nullptr;
	SpawnDataTable = nullptr;
	TimeAdding = 0.0f;
	RotationAmount = 60.0f;
	BounceAmount = 300.0f;

	ActivationCollision->OnComponentBeginOverlap.AddDynamic(this, &ABulletChargeItem::OnItemOverlap);
}

void ABulletChargeItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeAdding += DeltaTime;
	float ItemZLocation = FMath::Sin(TimeAdding) * BounceAmount;
	StaticMesh->AddRelativeRotation(FRotator(0.0f, RotationAmount, 0.0f) * DeltaTime);
	StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, ItemZLocation) * DeltaTime);
}

void ABulletChargeItem::OnItemOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && Cast<ACharacter>(OtherActor) == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		ActivateItem(OtherActor);
	}
}

void ABulletChargeItem::ActivateItem(AActor* Activator)
{
	if (FItemSpawnRow* SpawnRow = GetRandomItem())
	{
		if (AProPlayerCharacter* PlayerCharacter = Cast<AProPlayerCharacter>(Activator))
		{
			PlayerCharacter->InventoryManager->AddItemStackCount(SpawnRow->SpawnDefinition, SpawnRow->BulletAmount);
		}
	}

	if (PickUpNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			PickUpNiagaraSystem,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	/*UParticleSystemComponent* Particle = nullptr;

	if (PickUpParticle)
	{
		Particle = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			PickUpParticle,
			GetActorLocation(),
			GetActorRotation(),
			true
		);
	}

	if (Particle)
	{
		FTimerHandle DestroyParticleTimerHandle;
		TWeakObjectPtr<UParticleSystemComponent> WeakParticle = Particle;

		GetWorld()->GetTimerManager().SetTimer(DestroyParticleTimerHandle,
			[WeakParticle]()
			{
				if (WeakParticle.IsValid())
				{
					WeakParticle->DestroyComponent();
				}
			},
			1.0f, false);
	}*/

	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			PickUpSound,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	DestroyItem();
}

void ABulletChargeItem::DestroyItem()
{
	Destroy();
}

FItemSpawnRow* ABulletChargeItem::GetRandomItem() const
{
	if (!SpawnDataTable)
	{
		return nullptr;
	}

	TArray<FItemSpawnRow*> AllRows;
	static const FString ContextString(TEXT("ItemSpawnContext"));
	SpawnDataTable->GetAllRows(ContextString, AllRows);

	if (AllRows.IsEmpty()) return nullptr;

	float TotalChance = 0.0f;
	for (const FItemSpawnRow* Row : AllRows)
	{
		if (Row)
		{
			TotalChance += Row->SpawnChance;
		}
	}

	const float RandValue = FMath::FRandRange(0.0f, TotalChance);
	float AccumulateChance = 0.0f;

	for (FItemSpawnRow* Row : AllRows)
	{
		AccumulateChance += Row->SpawnChance;
		if (RandValue <= AccumulateChance)
		{
			return Row;
		}
	}

	return nullptr;
}
