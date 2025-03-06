#include "System/Acotrs/TrapActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// GAS 관련 헤더(프로젝트 구조에 따라 변경 필요)
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

// Niagara 스폰 함수 라이브러리
#include "AbilitySystemInterface.h"
#include "NiagaraFunctionLibrary.h"

ATrapActor::ATrapActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 콜리전 컴포넌트 생성
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATrapActor::OnTrapOverlap);
}

void ATrapActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATrapActor::OnTrapOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// 예: ACharacter만 해당 함정 효과를 받도록 할 경우
	ACharacter* OverlappedCharacter = Cast<ACharacter>(OtherActor);
	if (!OverlappedCharacter)
	{
		return;
	}

	// 여기서는 세 함수를 한 번에 호출하되, 필요한 인자를 넘겨줍니다.
	PlayTrapSound();
	SpawnNiagaraFX();
	ApplyTrapEffect(OverlappedCharacter);

	// 일회용 함정인 경우 파괴하거나, 쿨다운 로직 추가 등 원하는 처리를 수행
	// Destroy();
}

/** 함정 사운드 재생 */
void ATrapActor::PlayTrapSound()
{
	if (TrapSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, TrapSound, GetActorLocation());
	}
}

/** 나이아가라 이펙트 스폰 */
void ATrapActor::SpawnNiagaraFX()
{
	if (FireNiagaraSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FireNiagaraSystem,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

/** 대상 액터에게 함정 GameplayEffect 적용 */
void ATrapActor::ApplyTrapEffect(AActor* TargetActor)
{
	if (!TrapGameplayEffect || !TargetActor)
	{
		return;
	}

	// IAbilitySystemInterface를 가진 액터인지 확인
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(TargetActor);
	if (!AbilitySystemInterface)
	{
		return;
	}

	UAbilitySystemComponent* ASC = AbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
		TrapGameplayEffect,
		1.0f, // GameplayEffect 레벨
		EffectContextHandle
	);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
