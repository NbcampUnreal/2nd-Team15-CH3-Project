#include "AI/Actors/ProjectileAOEActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Components/SphereComponent.h"

AProjectileAOEActor::AProjectileAOEActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	AOETriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AOETriggerSphere"));
	AOETriggerSphere->InitSphereRadius(200.f);
	AOETriggerSphere->SetCollisionProfileName(TEXT("OverlapAll")); // 필요에 따라 커스텀
	AOETriggerSphere->SetupAttachment(RootComponent);
	
	AoeDuration = 5.f; // 5초 후 AoE 제거
}

void AProjectileAOEActor::BeginPlay()
{
	Super::BeginPlay();
	// Overlap 이벤트 바인딩
	AOETriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileAOEActor::OnOverlapBegin);

	// 일정 시간 후 파괴
	GetWorldTimerManager().SetTimer(AoeTimerHandle, this, &AProjectileAOEActor::DestroyAOE, AoeDuration, false);
}

void AProjectileAOEActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                         int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// AbilitySystem이 있는 Pawn/Actor라면 Debuff 적용
	if (IAbilitySystemInterface* ASInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		UAbilitySystemComponent* TargetASC = ASInterface->GetAbilitySystemComponent();
		// 여기서는 Source 없이 자기 자신이 소스라고 가정
		if (TargetASC && AOEEffectClass)
		{
			FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
			ContextHandle.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(AOEEffectClass, 1.f, ContextHandle);
			if (SpecHandle.IsValid())
			{
				TargetASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void AProjectileAOEActor::DestroyAOE()
{
	Destroy();
}
