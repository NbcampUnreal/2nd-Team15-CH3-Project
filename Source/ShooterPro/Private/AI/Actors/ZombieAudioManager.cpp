#include "AI/Actors/ZombieAudioManager.h"
#include "AI/AIGameplayTags.h"
#include "AI/EnemyAIBase.h"
#include "AI/EnemyAIController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" // 디버그 라인에 필요

AZombieAudioManager::AZombieAudioManager()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxConcurrentMoans = 3;
	MaxMoanDistance = 2000.f;
	bUseLineTrace = false;
    ZombieMoanCooldown = 3.f;
}

void AZombieAudioManager::BeginPlay()
{
	Super::BeginPlay();
    SetNextMoanTimer();
}

void AZombieAudioManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 타이머 해제
	GetWorldTimerManager().ClearTimer(MoanTimerHandle);

    // 재생 중인 AudioComponent 정리
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

    // 끝난 AudioComponent 제거
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

    // 등록 시점에 쿨타임 맵에도 기본값(0) 세팅
	ZombieToNextMoanTime.FindOrAdd(ZombieActor) = 0.f;
}

/** 좀비 해제 */
void AZombieAudioManager::UnregisterZombie(AActor* ZombieActor)
{
	if (!ZombieActor) return;
	RegisteredZombies.RemoveSingleSwap(ZombieActor, false);
	ZombieToNextMoanTime.Remove(ZombieActor);
}

/** 타이머 콜백 */
void AZombieAudioManager::HandleZombieMoan()
{
	UWorld* World = GetWorld();
    if (!World) return;

    // (1) 현재 재생 중인 소리와 MaxConcurrentMoans 비교
    int32 CurrentMoans = ActiveMoans.Num();
    if (CurrentMoans >= MaxConcurrentMoans)
	{
        // 이미 한도에 도달
        SetNextMoanTimer();
		return;
	}

    // (2) 플레이어 주변 + 쿨타임 끝난 좀비만 후보로 모으기
	TArray<AActor*> ZombiesInRange;
	for (AActor* Zombie : RegisteredZombies)
	{
		if (!Zombie)
			continue;

        float NowTime = World->GetTimeSeconds();
        float* NextMoanTimePtr = ZombieToNextMoanTime.Find(Zombie);
        if (NextMoanTimePtr && *NextMoanTimePtr > NowTime)
        {
            // 아직 쿨타임 중이면 제외
			continue;
        }

        // 거리/LineTrace 체크
        APawn* ClosestPlayer = GetClosestPlayerPawn(Zombie->GetActorLocation());
		if (!ClosestPlayer)
			continue;

        float Dist = FVector::Dist(Zombie->GetActorLocation(), ClosestPlayer->GetActorLocation());
		if (Dist > MaxMoanDistance)
		{
			continue;
		}

		if (bUseLineTrace)
		{
			FHitResult Hit;
            FCollisionQueryParams Params(SCENE_QUERY_STAT(ZombieAudioLineTrace), false, Zombie);

			bool bHit = World->LineTraceSingleByChannel(
				Hit,
                Zombie->GetActorLocation(),
				ClosestPlayer->GetActorLocation(),
				ECC_Visibility,
				Params
			);

			if (bHit && Hit.GetActor() != ClosestPlayer)
			{
                // 장애물이 막으면 제외
				continue;
			}

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
            DrawDebugLine(
                World, 
                Zombie->GetActorLocation(),
			              ClosestPlayer->GetActorLocation(),
                FColor::Green, 
                false, 1.f, 0, 2.f
            );
#endif
		}

        // 조건 통과 → 후보 배열에 추가
		ZombiesInRange.Add(Zombie);
	}

    // (3) 이제 “추가로 재생 가능한 소리 수”를 구함
    int32 AllowedMoans = MaxConcurrentMoans - CurrentMoans;
	AllowedMoans = FMath::Min(AllowedMoans, ZombiesInRange.Num());
	if (AllowedMoans <= 0)
	{
		SetNextMoanTimer();
		return;
	}

    // (4) 부분 셔플(Partial Shuffle)로 무작위 좀비 AllowedMoans마리 선택
    int32 CountPlayed = 0;
    // i=0부터 AllowedMoans 직전까지 '한 칸씩' 무작위 교환
    for (int32 i = 0; i < AllowedMoans; i++)
		{
        // i ~ ZombiesInRange.Num()-1 사이에서 랜덤 인덱스 뽑기
        int32 RandomIndex = FMath::RandRange(i, ZombiesInRange.Num() - 1);
        // Swap
        ZombiesInRange.Swap(i, RandomIndex);

        // 그리고 i번째 원소에 대해 사운드 재생 시도
        if (TryPlayMoan(ZombiesInRange[i]))
			{
            CountPlayed++;
		}
	}

	SetNextMoanTimer();
}

/** 실제로 좀비에게 사운드를 재생 시도 (AI 상태, 사운드 목록, 쿨타임 등록 등) */
bool AZombieAudioManager::TryPlayMoan(AActor* ZombieActor)
{
	if (!ZombieActor)
		return false;

	AEnemyAIBase* ZombieCharacter = Cast<AEnemyAIBase>(ZombieActor);
	if (!ZombieCharacter)
		return false;

	AEnemyAIController* ZController = Cast<AEnemyAIController>(ZombieCharacter->GetController());
	if (!ZController)
		return false;

	// 현재 AI 상태
    FGameplayTag CurrentState = ZController->GetCurrentStateTag();

    // 상태 -> 사운드 목록 찾기
	const FZombieSoundSet* SoundSetPtr = StateToSoundsMap.Find(CurrentState);
	if (!SoundSetPtr || SoundSetPtr->Sounds.Num() == 0)
	{
		return false;
	}

    // 사운드 무작위 선택
	USoundBase* RandomSound = SoundSetPtr->Sounds[FMath::RandRange(0, SoundSetPtr->Sounds.Num() - 1)];
	if (!RandomSound)
		return false;

    // 사운드 재생
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

	if (!AC)
		return false;

    // 동시재생 Concurrency
	for (USoundConcurrency* CObj : ConcurrencyList)
	{
		if (CObj)
		{
			AC->ConcurrencySet.Add(CObj);
		}
	}
	ActiveMoans.Add(AC);

    // 쿨타임 갱신 : (지금 시간 + ZombieMoanCooldown)
	float NowTime = GetWorld()->GetTimeSeconds();
    ZombieToNextMoanTime.FindOrAdd(ZombieActor) = NowTime + ZombieMoanCooldown;

	return true;
}

/** 상태별 (Min,Max) 간격으로 랜덤 딜레이 타이머 설정 */
void AZombieAudioManager::SetNextMoanTimer()
{
	if (!GetWorld())
		return;

    // 일단 Idle 기준 예시
	float MinTime = 5.f;
	float MaxTime = 10.f;

    // AIState_Idle 태그에 대한 설정이 있으면 덮어씀
	if (const FVector2D* IntervalRange = StateToIntervalMap.Find(AIGameplayTags::AIState_Idle))
	{
		MinTime = IntervalRange->X;
		MaxTime = IntervalRange->Y;
	}

	float NextDelay = FMath::FRandRange(MinTime, MaxTime);
	GetWorldTimerManager().SetTimer(MoanTimerHandle, this, &AZombieAudioManager::HandleZombieMoan, NextDelay, false);
}

/** 가장 가까운 플레이어 Pawn 찾기 */
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

        float DistSq = FVector::DistSquared(FromLocation, Pawn->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Pawn;
		}
	}

	return Closest;
}
