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

	// 블루프린트 확장을 위해 K2_OnTrapOverlap 호출
	K2_OnTrapOverlap(OverlappedComp,OtherActor,OtherComp,OtherBodyIndex,bFromSweep,SweepResult);

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

	// 오버랩 시 트랩이 사라지는 로직
	if (bDestroyOnOverlap)
	{
		// DestroyDelay가 0보다 크면 해당 시간 후 삭제
		if (DestroyDelay > 0.f)
		{
			SetLifeSpan(DestroyDelay); 
			// 또는 타이머를 쓰고 싶다면 GetWorldTimerManager().SetTimer() 방식도 가능
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
