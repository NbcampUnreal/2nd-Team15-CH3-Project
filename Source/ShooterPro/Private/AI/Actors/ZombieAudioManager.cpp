// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Actors/ZombieAudioManager.h"

#include "AI/AIGameplayTags.h"
#include "AI/EnemyAIBase.h"
#include "AI/EnemyAIController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"


AZombieAudioManager::AZombieAudioManager()
{
	PrimaryActorTick.bCanEverTick = true;
	MaxConcurrentMoans = 3;
	MaxMoanDistance = 2000.f;
	bUseLineTrace = false;
}

void AZombieAudioManager::BeginPlay()
{
	Super::BeginPlay();
	SetNextMoanTimer();
}

void AZombieAudioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(MoanTimerHandle);

	// 남아있는 AudioComponent 정리
	for (UAudioComponent* AC : ActiveMoans)
	{
		if (AC)
		{
			AC->Stop();
		}
	}
	ActiveMoans.Empty();

	Super::EndPlay(EndPlayReason);
}

void AZombieAudioManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이미 재생 종료된 AudioComponent 제거
	for (int32 i = ActiveMoans.Num() - 1; i >= 0; i--)
	{
		if (!ActiveMoans[i] || !ActiveMoans[i]->IsPlaying())
		{
			ActiveMoans.RemoveAt(i);
		}
	}
}

/** 좀비 등록 */
void AZombieAudioManager::RegisterZombie(AActor* ZombieActor)
{
	if (!ZombieActor) return;
	RegisteredZombies.AddUnique(ZombieActor);
}

/** 좀비 해제 */
void AZombieAudioManager::UnregisterZombie(AActor* ZombieActor)
{
	if (!ZombieActor) return;
	RegisteredZombies.RemoveSingleSwap(ZombieActor, false);
}

/** 타이머 콜백 */
void AZombieAudioManager::HandleZombieMoan()
{
	if (ActiveMoans.Num() < MaxConcurrentMoans)
	{
		PlayRandomMoanFromZombie();
	}

	SetNextMoanTimer();
}

/** 상태에 따라 (Min,Max) 간격을 구해 랜덤 시간으로 타이머 설정 */
void AZombieAudioManager::SetNextMoanTimer()
{
	if (!GetWorld())
		return;

	// 기본 Idle 간격 (5~10)
	float MinTime = 5.f;
	float MaxTime = 10.f;

	// AIState_Idle 태그 (가령)
	if (const FVector2D* IntervalRange = StateToIntervalMap.Find(AIGameplayTags::AIState_Idle))
	{
		MinTime = IntervalRange->X;
		MaxTime = IntervalRange->Y;
	}

	float NextDelay = FMath::FRandRange(MinTime, MaxTime);
	GetWorldTimerManager().SetTimer(MoanTimerHandle, this, &AZombieAudioManager::HandleZombieMoan, NextDelay, false);
}

/** 랜덤 좀비 → 좀비의 AI컨트롤러 → GetCurrentStateTag → 사운드 재생 */
void AZombieAudioManager::PlayRandomMoanFromZombie()
{
	if (RegisteredZombies.Num() == 0)
	{
		return;
	}

	// 1) 무작위로 한 좀비를 고름
	AActor* ChosenZombie = RegisteredZombies[FMath::RandRange(0, RegisteredZombies.Num() - 1)];
	if (!ChosenZombie) return;

	AEnemyAIBase* ZombieCharacter = Cast<AEnemyAIBase>(ChosenZombie);
	if (!ZombieCharacter) return;

	// 2) AI Controller에서 GetCurrentStateTag() 호출
	AEnemyAIController* ZController = Cast<AEnemyAIController>(ZombieCharacter->GetController());
	if (!ZController) return;

	const FGameplayTag CurrentState = ZController->GetCurrentStateTag();

	// 3) 해당 State에 대응하는 사운드 목록(FZombieSoundSet)을 찾음
	const FZombieSoundSet* SoundSetPtr = StateToSoundsMap.Find(CurrentState);
	if (!SoundSetPtr || SoundSetPtr->Sounds.Num() == 0)
	{
		// 해당 상태에 사운드가 없다면 중단
		return;
	}

	// 4) 거리/LineTrace 체크
	APawn* ClosestPlayer = GetClosestPlayerPawn(ZombieCharacter->GetActorLocation());
	if (!ClosestPlayer)
		return;

	const float Dist = FVector::Dist(ZombieCharacter->GetActorLocation(), ClosestPlayer->GetActorLocation());
	if (Dist > MaxMoanDistance)
	{
		return;
	}

	if (bUseLineTrace)
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ZombieAudioLineTrace), false, ZombieCharacter);
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			Hit,
			ZombieCharacter->GetActorLocation(),
			ClosestPlayer->GetActorLocation(),
			ECC_Visibility,
			Params
		);

		if (bHit && Hit.GetActor() != ClosestPlayer)
		{
			// 장애물 있음
			return;
		}
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
		DrawDebugLine(GetWorld(), ZombieCharacter->GetActorLocation(), ClosestPlayer->GetActorLocation(), FColor::Green, false, 1.f, 0, 2.f);
#endif
	}

	// 5) 사운드 하나 랜덤 선택 후 재생
	USoundBase* RandomSound = SoundSetPtr->Sounds[FMath::RandRange(0, SoundSetPtr->Sounds.Num() - 1)];
	if (!RandomSound)
		return;

	// SpawnSoundAtLocation(Concurrency 파라미터는 null),
	// AudioComponent를 반환받아 직접 ConcurrencySet에 삽입
	UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(
		this,
		RandomSound,
		ZombieCharacter->GetActorLocation(),
		FRotator::ZeroRotator,
		1.f,
		1.f,
		0.f,
		nullptr,
		nullptr,
		true // bAutoDestroy
	);

	if (AC)
	{
		for (USoundConcurrency* CObj : ConcurrencyList)
		{
			if (CObj)
			{
				AC->ConcurrencySet.Add(CObj);
			}
		}
		ActiveMoans.Add(AC);
	}
}


APawn* AZombieAudioManager::GetClosestPlayerPawn(const FVector& FromLocation) const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	APawn* Closest = nullptr;
	float ClosestDistSq = FLT_MAX;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* Pawn = PC->GetPawn();
		if (!Pawn) continue;

		const float DistSq = FVector::DistSquared(FromLocation, Pawn->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Pawn;
		}
	}

	return Closest;
}
