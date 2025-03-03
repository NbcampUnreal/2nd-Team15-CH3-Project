#include "AI/Actors/EnemyProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AI/EnemyAIController.h"
#include "AI/Actors/ProjectileAOEActor.h"
#include "AI/Interfaces/Interface_EnemyAI.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

#include "Kismet/GameplayStatics.h"


AEnemyProjectile::AEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// 1) 충돌체
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(15.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionSphere;

	// 2) (필요 없다면 제거 가능) 시각적 메시
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3) 이동 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

	// 4) 날아가는 토사물 나이아가라 컴포넌트
	ProjectileVomitNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileVomitNiagara"));
	ProjectileVomitNiagara->SetupAttachment(RootComponent);
	ProjectileVomitNiagara->bAutoActivate = true; // 코드로 수동 Activate

	// 자동 제거 시간
	InitialLifeSpan = 10.0f;

	// 충돌 이벤트
	CollisionSphere->OnComponentHit.AddDynamic(this, &AEnemyProjectile::OnHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyProjectile::OnOverlapBegin);
}

void AEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 투사체가 생성되면, "날아가는 토사물" 나이아가라 이펙트를 세팅 & 재생
	if (FlyingVomitNiagara)
	{
		ProjectileVomitNiagara->SetAsset(FlyingVomitNiagara);
		ProjectileVomitNiagara->Activate(true);
	}
}

void AEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                             FVector NormalImpulse, const FHitResult& Hit)
{
	AActor* MyInstigator = GetInstigator();
	if (OtherActor && OtherActor != this && OtherActor != MyInstigator)
	{
		// 벽/바닥 충돌 시
		if (ImpactEffectWorld)
		{
			// Niagara에서는 SpawnEmitterAtLocation 대신 SpawnSystemAtLocation 사용
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				ImpactEffectWorld,
				Hit.ImpactPoint,
				Hit.ImpactNormal.Rotation(), ImpactScale
			);
		}

		// 투사체 제거
		Destroy();
	}


	// // AoE 생성
	// if (AOEActorClass)
	// {
	// 	GetWorld()->SpawnActor<AProjectileAOEActor>(AOEActorClass, Hit.ImpactPoint, FRotator::ZeroRotator);
	// }
}

void AEnemyProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                      int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* MyInstigator = GetInstigator();


	if (OtherActor && OtherActor != this && OtherActor != MyInstigator)
	{
		AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(MyInstigator));
		if (EnemyAIController->OnSameTeam(OtherActor))
		{
			return;
		}

		// 플레이어(또는 적 NPC)에 피격 시 이펙트
		if (ImpactEffectPlayer)
		{
			// Niagara 에서는 SpawnSystemAttached 사용
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				ImpactEffectPlayer,
				OtherActor->GetRootComponent(),
				FName("spine_03"), // 본/소켓명 (필요에 따라 "")
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTargetIncludingScale,
				true // bAutoDestroy
			);

			// 디버프 (GAS)
			if (IAbilitySystemInterface* ASInterface = Cast<IAbilitySystemInterface>(OtherActor))
			{
				UAbilitySystemComponent* TargetASC = ASInterface->GetAbilitySystemComponent();
				UAbilitySystemComponent* SourceASC = nullptr;

				if (IAbilitySystemInterface* SourceASInterface = Cast<IAbilitySystemInterface>(MyInstigator))
				{
					SourceASC = SourceASInterface->GetAbilitySystemComponent();
				}

				if (TargetASC && SourceASC && DeBuffEffectClass)
				{
					FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
					ContextHandle.AddSourceObject(this);

					FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
						DeBuffEffectClass,
						1.0f,
						ContextHandle
					);
					if (SpecHandle.IsValid())
					{
						SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
					}
				}
			}
		}


		// 제거
		Destroy();
	}
}
