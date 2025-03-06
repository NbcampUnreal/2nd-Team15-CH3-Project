// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ProBulletBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
AProBulletBase::AProBulletBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 충돌체 초기화 (구체 콜리전)
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AProBulletBase::OnHit);
	RootComponent = CollisionComp;

	// 프로젝트타일 무브먼트 컴포넌트 초기화
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->bRotationFollowsVelocity = true; // 이동 방향으로 회전
	ProjectileMovement->bInitialVelocityInLocalSpace = false; // 초기 속도를 월드 방향으로 적용
	ProjectileMovement->ProjectileGravityScale = 0.0f; // 중력 영향 (1.0 = 기본 중력, 0.0 = 중력 무시)
	
	ProjectileMovement->InitialSpeed = 2000.f; // 초기 속도
	ProjectileMovement->MaxSpeed = 2000.f; // 최대 속도 (감속이 없는 경우 InitialSpeed와 동일하게 설정)

	InitialLifeSpan = 0.0f; //오브젝트 풀링 방식을 사용할 것이기 때문에 0.0f
}

// Called when the game starts or when spawned
void AProBulletBase::BeginPlay()
{
	Super::BeginPlay();
}

void AProBulletBase::ActivateBullet(AActor* Avatar, const FVector& SpawnLocation, const FRotator& SpawnRotation, const FVector& Direction, const float Speed)
{
	//Activate(Avatar, SpawnLocation, SpawnRotation, Direction, Speed);
	SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	AvatarActor = Avatar;
	
	// ProjectileMovement를 재설정
	ProjectileMovement->Velocity = Direction * Speed; // 원하는 방향 * 속도
	ProjectileMovement->Activate(); // 혹시나 비활성화되었을 수도 있으므로 활성화
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	
	CollisionComp->IgnoreActorWhenMoving(AvatarActor, true);

	K2_ActivateBullet(Avatar, SpawnLocation, SpawnRotation, Direction, Speed);
}

void AProBulletBase::DeactivateBullet()
{
	// 눈에 안 보이도록, 충돌도 중단
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	
	// 여기서 "풀 매니저에게 반환"을 해도 되지만,
	// 보통은 탄환이 자신만 알기보다 "매니저 호출" 함수를 통해
	// BulletPoolManager가 ReleaseBullet(this)를 호출하도록 하는 편이 낫습니다.
}

void AProBulletBase::OnBounce_Implementation(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
}


void AProBulletBase::OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor) return;

	if (OtherActor == AvatarActor) return;
	
	// 1. 데미지 적용 (GameplayEffect 적용)
	if (DamageEffect && OtherActor)
	{
		// 대상의 AbilitySystem 컴포넌트 가져오기
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor))
		{
			// 이펙트Spec 생성
			FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);
			FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffect, /*Level=*/1, EffectContext);

			// 효과 적용
			if (SpecHandle.IsValid())
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	
	DeactivateBullet();
}
