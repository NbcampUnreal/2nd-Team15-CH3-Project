#include "System/ZombieAudioManager.h"

#include "AI/EnemyAIBase.h"
#include "AI/EnemyAIController.h"
#include "AI/AIGameplayTags.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AZombieAudioManager::AZombieAudioManager()
{
	PrimaryActorTick.bCanEverTick = true;

	MaxConcurrentMoans = 3;
	MaxMoanDistance = 2000.f;
	bUseLineTrace = false;
	ZombieMoanCooldown = 3.f;

	// 좀비 사운드 전체 볼륨(0.0~1.0), 기본값 1.0 (최대)
	ZombieMoanVolume = 1.0f;
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
	for (int32 i = 0; i < ActiveMoansData.Num(); i++)
	{
		if (ActiveMoansData[i].AudioComp)
		{
			ActiveMoansData[i].AudioComp->Stop();
		}
	}
	ActiveMoansData.Empty();

	Super::EndPlay(EndPlayReason);
}

void AZombieAudioManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World) return;

	// 끝난 오디오, 거리 벗어난 오디오 등 정리
	for (int32 i = ActiveMoansData.Num() - 1; i >= 0; i--)
	{
		FZombieMoanData& MoanData = ActiveMoansData[i];
		UAudioComponent* AC = MoanData.AudioComp;

		// AudioComponent가 이미 사라졌거나, 재생이 끝났다면 제거
		if (!AC || !AC->IsPlaying())
		{
			ActiveMoansData.RemoveAt(i);
			continue;
		}

		// 해당 사운드를 낸 좀비가 유효한지
		AActor* ZombieActor = MoanData.ZombieActor.Get();
		if (!ZombieActor)
		{
			AC->Stop();
			ActiveMoansData.RemoveAt(i);
			continue;
		}

		// 플레이어 거리 검사
		APawn* ClosestPlayer = GetClosestPlayerPawn(ZombieActor->GetActorLocation());
		if (!ClosestPlayer)
		{
			AC->Stop();
			ActiveMoansData.RemoveAt(i);
			continue;
		}

		float Dist = FVector::Dist(ZombieActor->GetActorLocation(), ClosestPlayer->GetActorLocation());
		if (Dist > MaxMoanDistance)
		{
			AC->Stop();
			ActiveMoansData.RemoveAt(i);
			continue;
		}
	}
}

void AZombieAudioManager::RegisterZombie(AActor* ZombieActor)
{
	if (!ZombieActor) return;

	RegisteredZombies.AddUnique(ZombieActor);
	ZombieToNextMoanTime.FindOrAdd(ZombieActor) = 0.f;
}

void AZombieAudioManager::UnregisterZombie(AActor* ZombieActor)
{
	if (!ZombieActor) return;

	RegisteredZombies.RemoveSingleSwap(ZombieActor, false);
	ZombieToNextMoanTime.Remove(ZombieActor);
}

void AZombieAudioManager::HandleZombieMoan()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 현재 재생 중인 수 확인
	int32 CurrentMoans = ActiveMoansData.Num();
	if (CurrentMoans >= MaxConcurrentMoans)
	{
		SetNextMoanTimer();
		return;
	}

	// 후보군 모으기
	TArray<AActor*> ZombiesInRange;
	for (AActor* Zombie : RegisteredZombies)
	{
		if (!Zombie) continue;

		float NowTime = World->GetTimeSeconds();
		float* NextMoanTimePtr = ZombieToNextMoanTime.Find(Zombie);
		if (NextMoanTimePtr && *NextMoanTimePtr > NowTime)
		{
			// 쿨타임 중
			continue;
		}

		APawn* ClosestPlayer = GetClosestPlayerPawn(Zombie->GetActorLocation());
		if (!ClosestPlayer)
			continue;

		float Dist = FVector::Dist(Zombie->GetActorLocation(), ClosestPlayer->GetActorLocation());
		if (Dist > MaxMoanDistance)
			continue;

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

		// 조건 통과
		ZombiesInRange.Add(Zombie);
	}

	// 추가로 재생 가능한 수
	int32 AllowedMoans = MaxConcurrentMoans - CurrentMoans;
	AllowedMoans = FMath::Min(AllowedMoans, ZombiesInRange.Num());
	if (AllowedMoans <= 0)
	{
		SetNextMoanTimer();
		return;
	}

	// 무작위 셔플 + 재생
	for (int32 i = 0; i < AllowedMoans; i++)
	{
		int32 RandomIndex = FMath::RandRange(i, ZombiesInRange.Num() - 1);
		ZombiesInRange.Swap(i, RandomIndex);

		if (TryPlayMoan(ZombiesInRange[i]))
		{
			// CountPlayed++ (필요시)
		}
	}

	SetNextMoanTimer();
}

bool AZombieAudioManager::TryPlayMoan(AActor* ZombieActor)
{
	if (!ZombieActor) return false;

	AEnemyAIBase* ZombieCharacter = Cast<AEnemyAIBase>(ZombieActor);
	if (!ZombieCharacter) return false;

	AEnemyAIController* ZController = Cast<AEnemyAIController>(ZombieCharacter->GetController());
	if (!ZController) return false;

	EAIState CurrentState = ZController->GetCurrentState();

	// 사운드 목록 찾기
	const FZombieSoundSet* SoundSetPtr = StateToSoundsMap.Find(CurrentState);
	if (!SoundSetPtr || SoundSetPtr->Sounds.Num() == 0)
		return false;

	// 무작위 사운드 선택
	USoundBase* RandomSound = SoundSetPtr->Sounds[FMath::RandRange(0, SoundSetPtr->Sounds.Num() - 1)];
	if (!RandomSound)
		return false;

	// 사운드 재생: 볼륨 = ZombieMoanVolume
	UAudioComponent* AC = UGameplayStatics::SpawnSoundAtLocation(
		this,
		RandomSound,
		ZombieCharacter->GetActorLocation(),
		FRotator::ZeroRotator,
		/* VolumeMultiplier */ ZombieMoanVolume,
		/* PitchMultiplier */ 1.f,
		/* StartTime */ 0.f,
		nullptr,
		nullptr,
		/* bAutoDestroy */ true
	);
	if (!AC)
		return false;

	// Concurrency 적용
	for (USoundConcurrency* CObj : ConcurrencyList)
	{
		if (CObj)
		{
			AC->ConcurrencySet.Add(CObj);
		}
	}

	// 추적 리스트에 추가
	FZombieMoanData NewMoanData;
	NewMoanData.AudioComp = AC;
	NewMoanData.ZombieActor = ZombieActor;
	ActiveMoansData.Add(NewMoanData);

	// 쿨타임 갱신
	float NowTime = GetWorld()->GetTimeSeconds();
	ZombieToNextMoanTime.FindOrAdd(ZombieActor) = NowTime + ZombieMoanCooldown;

	return true;
}

void AZombieAudioManager::SetNextMoanTimer()
{
	if (!GetWorld()) return;

	// 예: Idle 상태 기준으로 (5,10)초 로직
	float MinTime = 5.f;
	float MaxTime = 10.f;

	if (const FVector2D* IntervalRange = StateToIntervalMap.Find(EAIState::Idle))
	{
		MinTime = IntervalRange->X;
		MaxTime = IntervalRange->Y;
	}

	float NextDelay = FMath::FRandRange(MinTime, MaxTime);
	GetWorldTimerManager().SetTimer(MoanTimerHandle, this, &AZombieAudioManager::HandleZombieMoan, NextDelay, false);
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

		float DistSq = FVector::DistSquared(FromLocation, Pawn->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			Closest = Pawn;
		}
	}

	return Closest;
}

/** 좀비 소리 전체 볼륨을 변경(0.0 ~ 1.0)하고, 이미 재생 중인 사운드도 업데이트 */
void AZombieAudioManager::SetZombieMoanVolume(float InVolume)
{
	// 볼륨 값 보정 (0.0 ~ 1.0 사이)
	ZombieMoanVolume = FMath::Clamp(InVolume, 0.f, 1.f);

	// 이미 재생 중인 사운드에 즉시 반영
	for (FZombieMoanData& MoanData : ActiveMoansData)
	{
		if (MoanData.AudioComp && MoanData.AudioComp->IsPlaying())
		{
			// SetVolumeMultiplier로 현재 재생 중인 소리도 볼륨 변경
			MoanData.AudioComp->SetVolumeMultiplier(ZombieMoanVolume);
		}
	}
}
