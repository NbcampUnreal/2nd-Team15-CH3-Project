// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/BulletChargeItem.h"
#include "Character/Player/ProPlayerCharacter.h"
#include "Inventory/InventoryManagerComponent.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ABulletChargeItem::ABulletChargeItem()
{
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	ActivationCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ActivationCollision"));
	ActivationCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ActivationCollision->SetSphereRadius(100.0f);
	ActivationCollision->SetupAttachment(Scene);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->SetupAttachment(Scene);

	PickUpParticle = nullptr;
	PickUpSound = nullptr;
	ItemToCharge = nullptr;
	StackToCharge = 6;

	ActivationCollision->OnComponentBeginOverlap.AddDynamic(this, &ABulletChargeItem::OnItemOverlap);
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
	ensure(StackToCharge > 0);
	if (AProPlayerCharacter* PlayerCharacter = Cast<AProPlayerCharacter>(Activator))
	{
		PlayerCharacter->InventoryManager->AddItemStackCount(ItemToCharge, StackToCharge);
	}
	
	UParticleSystemComponent* Particle = nullptr;

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
	}

	DestroyItem();
}

void ABulletChargeItem::DestroyItem()
{
	Destroy();
}

