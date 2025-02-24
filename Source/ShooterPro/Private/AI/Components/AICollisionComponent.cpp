#include "AI/Components/AICollisionComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


UAICollisionComponent::UAICollisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// ...
	// ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	// IgnoredActorTag.Add("Enemies");
	// DrawDebug = EDrawDebugTrace::None;
}

void UAICollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UAICollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//
	// if (!bCollisionActive)
	// 	return;
	//
	// if (bTraceActive)
	// 	ActivateTrace();
	//
	// UpdateLastSocketPositions();
	// bTraceActive = true;
}

// void UAICollisionComponent::EnableCollision(ECollisionPart CollisionPart)
// {
// 	// HitActors.Empty();
//
// 	bCollisionActive = true;
//
// 	// if (OnCollisionEnabled.IsBound())
// 	// 	OnCollisionEnabled.Broadcast(CollisionPart);
// }
//
// void UAICollisionComponent::DisableCollision()
// {
// 	bCollisionActive = false;
// 	bTraceActive = false;
//
// 	if (OnCollisionDisabled.IsBound())
// 		OnCollisionDisabled.Broadcast();
// }
//
// void UAICollisionComponent::SetCollisions(const TArray<FHitCollisionComponent>& NewHitCollisions)
// {
// 	// CollisionComponents = NewHitCollisions;
// 	UpdateLastSocketPositions();
// }
//
// void UAICollisionComponent::AddHitActor(UPrimitiveComponent* Component, AActor* HitActor)
// {
// 	int32 HitIndex = GetHitActorsIndex(Component);
// 	if (HitIndex >= 0)
// 	{
// 		// HitActors[HitIndex].HitActors.AddUnique(HitActor);
// 	}
// }
//
// void UAICollisionComponent::SpawnHitVFXAndSound(const FHitResult& HitResult, UParticleSystem* HitVFX, USoundBase* HitSound, FVector Scale)
// {
// 	UGameplayStatics::SpawnEmitterAtLocation(this, HitVFX, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator(), Scale);
// 	UGameplayStatics::SpawnSoundAtLocation(this, HitSound, HitResult.ImpactPoint);
// }
//
// void UAICollisionComponent::ApplyDamageToActor(const FHitResult& Hit, float BaseDamage)
// {
// 	FVector DirVector = UKismetMathLibrary::GetDirectionUnitVector(GetOwner()->GetActorLocation(), Hit.GetActor()->GetActorLocation());
// 	UGameplayStatics::ApplyPointDamage(Hit.GetActor(), BaseDamage, DirVector, Hit, nullptr, GetOwner(), nullptr);
// }
//
// void UAICollisionComponent::ActivateTrace()
// {
// 	// for (const FHitCollisionComponent& CollisionComponent : CollisionComponents)
// 	// {
// 	// 	if (!IsValid(CollisionComponent.Component))
// 	// 		continue;
// 	//
// 	// 	for (const FName& Socket : CollisionComponent.Sockets)
// 	// 	{
// 	// 		FName UniqueSocketName = GetUniqueSocketName(CollisionComponent.Component, Socket);
// 	// 		FVector Start = LastSocketPositions.Contains(UniqueSocketName) ? LastSocketPositions[UniqueSocketName] : CollisionComponent.Component->GetSocketLocation(Socket);
// 	// 		FVector End = CollisionComponent.Component->GetSocketLocation(Socket);
// 	// 		TArray<FHitResult> OutHits;
// 	// 		UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, TraceRadius, ObjectTypes, false, GetHitActors(CollisionComponent.Component), DrawDebug,
// 	// 		                                                 OutHits, true, FLinearColor::Red, FLinearColor::Green, DrawTime);
// 	//
// 	// 		for (const FHitResult& OutHit : OutHits)
// 	// 		{
// 	// 			AActor* HitActor = OutHit.GetActor();
// 	// 			if (GetHitActors(CollisionComponent.Component).Contains(HitActor))
// 	// 				continue;
// 	//
// 	// 			for (const FName& HitTag : HitActor->Tags)
// 	// 			{
// 	// 				if (IgnoredActorTag.Contains(HitTag))
// 	// 					continue;
// 	//
// 	// 				AddHitActor(CollisionComponent.Component, HitActor);
// 	//
// 	// 				if (OnHit.IsBound())
// 	// 					OnHit.Broadcast(OutHit);
// 	// 			}
// 	// 		}
// 	// 	}
// 	// }
// }
//
// void UAICollisionComponent::UpdateLastSocketPositions()
// {
// 	// for (const FHitCollisionComponent& CollisionComponent : CollisionComponents)
// 	// {
// 	// 	if (!IsValid(CollisionComponent.Component))
// 	// 		continue;
// 	//
// 	// 	for (const FName& Socket : CollisionComponent.Sockets)
// 	// 	{
// 	// 		FName UniqueSocketName = GetUniqueSocketName(CollisionComponent.Component, Socket);
// 	// 		LastSocketPositions.Add(UniqueSocketName, CollisionComponent.Component->GetSocketLocation(Socket));
// 	// 	}
// 	// }
// }
//
// FName UAICollisionComponent::GetUniqueSocketName(UPrimitiveComponent* Component, FName SocketName)
// {
// 	FString ComponentDisplayName = UKismetSystemLibrary::GetDisplayName(Component);
// 	ComponentDisplayName.Append(SocketName.ToString());
// 	return FName(ComponentDisplayName);
// }
//
// int32 UAICollisionComponent::GetHitActorsIndex(UPrimitiveComponent* Component)
// {
// 	// for (int32 i = 0; i < HitActors.Num(); i++)
// 	// {
// 	// 	if (HitActors[i].Component == Component)
// 	// 	{
// 	// 		return i;
// 	// 	}
// 	// }
// 	 return -1;
// }
//
// TArray<AActor*> UAICollisionComponent::GetHitActors(UPrimitiveComponent* Component)
// {
// 	// int32 HitIndex = GetHitActorsIndex(Component);
// 	//
// 	// if (HitIndex >= 0)
// 	// {
// 	// 	return HitActors[HitIndex].HitActors;
// 	// }
// 	//
// 	// TArray<AActor*> NewHitActors;
// 	// NewHitActors.Add(GetOwner());
// 	// int32 AddIndex = HitActors.Add(FHitActorCollisionComponent(Component, NewHitActors));
// 	// return HitActors[AddIndex].HitActors;
// 	return {};
// }
