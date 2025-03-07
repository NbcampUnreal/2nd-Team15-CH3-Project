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
#include "Character/Player/ProPlayerCharacter.h"

ATrapActor::ATrapActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	// 콜리전 컴포넌트 생성
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Overlap);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATrapActor::OnTrapOverlap);

	// 초기값 설정
	bDestroyOnOverlap = false;
	DestroyDelay = 0.f;
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

	// 필터 클래스가 설정되어 있다면, OverlapFilterClass를 상속하는 액터인지 확인
	if (OverlapFilterClass)
	{
		// OtherActor가 OverlapFilterClass(또는 자식 클래스)인지 체크
		if (!OtherActor->IsA(OverlapFilterClass))
		{
			// 필터 조건을 만족하지 않으면 트리거 중단
			return;
		}
	}

	// 만약 특정 상황에서 "플레이어만" 체크하고 싶다면 추가로 bOnlyPlayer 같은 조건 사용
	// if (bOnlyPlayer)
	// {
	//     AProPlayerCharacter* PlayerCharacter = Cast<AProPlayerCharacter>(OtherActor);
	//     if (!PlayerCharacter)
	//     {
	//         return;
	//     }
	// }

	// 블루프린트 확장을 위해 K2_OnTrapOverlap 호출
	K2_OnTrapOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// 여기서는 ACharacter만 함정 효과를 받는다고 예시로 두었지만,
	// 필요 없다면 제거하거나 OverlapFilterClass만 사용해도 됩니다.
	ACharacter* OverlappedCharacter = Cast<ACharacter>(OtherActor);
	if (!OverlappedCharacter)
	{
		return;
	}

	// 사운드, 이펙트, GameplayEffect 적용
	PlayTrapSound();
	SpawnNiagaraFX();
	ApplyTrapEffect(OverlappedCharacter);

	// 오버랩 시 트랩을 파괴하는 로직 (기존 코드)
	if (bDestroyOnOverlap)
	{
		if (DestroyDelay > 0.f)
		{
			SetLifeSpan(DestroyDelay);
		}
		else
		{
			Destroy();
		}
	}
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
