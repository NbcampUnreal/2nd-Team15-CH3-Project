#include "AI/AIDectionInfoTypes.h"

#include "AI/EnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"

void FPerceivedActorInfo::UpdateWithStimulus(const FAIStimulus& NewStimulus, float CurrentTime)
{
	if (NewStimulus.WasSuccessfullySensed())
	{
		bCurrentlySensed = true;
		bLostStimulus = false;
		LastSensedTime = CurrentTime;
		LastKnownLocation = NewStimulus.StimulusLocation;
		LastSensorLocation = NewStimulus.ReceiverLocation;
		// 최신 자극 데이터 동기화
		SenseData = NewStimulus;
	}
	else
	{
		if (bCurrentlySensed)
		{
			bLostStimulus = true;
		}
		bCurrentlySensed = false;
		SenseData = NewStimulus;
	}
}

UPerceptionManager::UPerceptionManager()
{
}

void UPerceptionManager::BeginDestroy()
{
	// 델리게이트 해제
	OnAddPerceptionUpdated.RemoveAll(this);
	OnRemoveExpiredDetection.RemoveAll(this);

	UObject::BeginDestroy();
}

void UPerceptionManager::AddOrUpdateDetection(AActor* Detector, AActor* DetectedActor, EAISense SenseType, const FAIStimulus& NewStimulus, float CurrentTime)
{
	if (!IsValid(DetectedActor))
		return;

	// 이미 해당 액터에 대한 감각 정보가 존재하는지 확인
	if (FPerceivedActorEntry* DetectionEntry = DetectionList.Find(DetectedActor))
	{
		// 감지된 액터에 대한 각 감각 정보를 순회
		for (int32 i = DetectionEntry->PerceivedActorInfos.Num() - 1; i >= 0; i--)
		{
			FPerceivedActorInfo& Info = DetectionEntry->PerceivedActorInfos[i];
			if (Info.DetectedSense == SenseType)
			{
				// 감각별로 새 자극을 비교하여 업데이트 여부 결정
				bool bShouldUpdate = false;

				// if (NewStimulus.WasSuccessfullySensed() && !Info.SenseData.WasSuccessfullySensed())
				if (NewStimulus.WasSuccessfullySensed() != Info.SenseData.WasSuccessfullySensed())
				{
					bShouldUpdate = true;
				}
				else if (NewStimulus.GetAge() < Info.SenseData.GetAge())
				{
					bShouldUpdate = true;
				}
				else if (NewStimulus.Strength > Info.SenseData.Strength)
				{
					bShouldUpdate = true;
				}

				if (bShouldUpdate)
				{
					// 감각 정보를 업데이트
					Info.Detector = Detector;
					Info.UpdateWithStimulus(NewStimulus, CurrentTime);

					FPerceivedActorInfo NewPerceivedActorInfo = Info;

					// 기존 항목을 지우고 새로운 항목 추가
					DetectionEntry->PerceivedActorInfos.RemoveAll([&Info](const FPerceivedActorInfo& Element)
					{
						return Element == Info;
					});

					DetectionEntry->PerceivedActorInfos.Add(NewPerceivedActorInfo); // 최신 정보 추가

					if (OnAddPerceptionUpdated.IsBound())
						OnAddPerceptionUpdated.Broadcast(Info); // 델리게이트 호출
					return;
				}
			}
		}

		// 감각 정보가 없으면 새로 추가
		FPerceivedActorInfo NewInfo;
		NewInfo.Detector = Detector;
		NewInfo.DetectedActor = DetectedActor;
		NewInfo.DetectedSense = SenseType;
		NewInfo.UpdateWithStimulus(NewStimulus, CurrentTime);

		// 새로운 항목을 추가
		DetectionEntry->PerceivedActorInfos.Add(NewInfo);

		if (OnAddPerceptionUpdated.IsBound())
			OnAddPerceptionUpdated.Broadcast(NewInfo); // 델리게이트 호출
	}
	else
	{
		// 해당 액터가 없으면 새로 생성하여 추가
		FPerceivedActorEntry NewEntry;
		FPerceivedActorInfo NewInfo;
		NewInfo.Detector = Detector;
		NewInfo.DetectedActor = DetectedActor;
		NewInfo.DetectedSense = SenseType;
		NewInfo.UpdateWithStimulus(NewStimulus, CurrentTime);
		NewEntry.PerceivedActorInfos.Add(NewInfo);
		DetectionList.Add(DetectedActor, NewEntry);

		if (OnAddPerceptionUpdated.IsBound())
			OnAddPerceptionUpdated.Broadcast(NewInfo); // 델리게이트 호출
	}
}


const FPerceivedActorEntry* UPerceptionManager::GetDetectionEntry(AActor* Actor) const
{
	if (DetectionList.Contains(Actor))
	{
		return &DetectionList[Actor];
	}

	return nullptr; // 액터가 존재하지 않으면 nullptr 반환
}


void UPerceptionManager::TickSenseDetectionsForActor(float DeltaTime, EAISense TickSense, const FAIStimulus& TickStimulus, AActor* RelevantActor)
{
	// 현재 시간을 가져옵니다
	float CurrentTime = (GetWorldFromSomewhere() != nullptr) ? GetWorldFromSomewhere()->GetTimeSeconds() : 0.f;

	if (FPerceivedActorEntry* DetectionEntry = DetectionList.Find(RelevantActor))
	{
		for (FPerceivedActorInfo& Info : DetectionEntry->PerceivedActorInfos)
		{
			if (Info.DetectedSense == TickSense)
			{
				Info.UpdateWithStimulus(TickStimulus, CurrentTime);
			}
		}
	}

	// 감지 정보 만료 처리
	RemoveExpiredDetectionInfos(CurrentTime);
}

void UPerceptionManager::RemoveExpiredDetectionInfos(float CurrentTime)
{
	for (auto It = DetectionList.CreateIterator(); It; ++It)
	{
		FPerceivedActorEntry& DetectionEntry = It.Value();
		for (int32 i = DetectionEntry.PerceivedActorInfos.Num() - 1; i >= 0; i--)
		{
			FPerceivedActorInfo& Info = DetectionEntry.PerceivedActorInfos[i];
			if (!IsValid(Info.DetectedActor) || Info.SenseData.IsExpired())
			{
				if (OnRemoveExpiredDetection.IsBound())
					OnRemoveExpiredDetection.Broadcast(Info);

				DetectionEntry.PerceivedActorInfos.RemoveAt(i);
			}
		}
		if (DetectionEntry.PerceivedActorInfos.Num() == 0)
			It.RemoveCurrent();
	}
}


void UPerceptionManager::ForgetActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	DetectionList.Remove(Actor);
}

void UPerceptionManager::GetCurrentlySensedActors(TArray<AActor*>& OutActors) const
{
	OutActors.Empty();

	for (const TTuple<AActor*, FPerceivedActorEntry>& Entry : DetectionList)
	{
		const FPerceivedActorEntry& DetectionEntry = Entry.Value;

		// 각 감지 정보에서 현재 감지된 액터들을 확인
		for (const FPerceivedActorInfo& Info : DetectionEntry.PerceivedActorInfos)
		{
			if (Info.bCurrentlySensed && IsValid(Info.DetectedActor))
			{
				OutActors.AddUnique(Info.DetectedActor); // 중복을 방지하여 추가
			}
		}
	}
}

UWorld* UPerceptionManager::GetWorldFromSomewhere() const
{
	AEnemyAIController* Controller = Cast<AEnemyAIController>(GetOuter());
	return Controller ? Controller->GetWorld() : nullptr;
}


void UPerceptionManager::GetAllDetectedActors(TArray<AActor*>& OutActors) const
{
	for (const TPair<AActor*, FPerceivedActorEntry>& Entry : DetectionList)
	{
		for (const FPerceivedActorInfo& Info : Entry.Value.PerceivedActorInfos)
		{
			if (Info.bCurrentlySensed && IsValid(Info.DetectedActor))
			{
				OutActors.AddUnique(Info.DetectedActor);
			}
		}
	}
}

void UPerceptionManager::GetDetectedActorsBySense(EAISense SenseType, TArray<AActor*>& OutActors) const
{
	for (const TPair<AActor*, FPerceivedActorEntry>& Entry : DetectionList)
	{
		for (const FPerceivedActorInfo& Info : Entry.Value.PerceivedActorInfos)
		{
			if (Info.DetectedSense == SenseType && Info.bCurrentlySensed && IsValid(Info.DetectedActor))
			{
				OutActors.AddUnique(Info.DetectedActor);
			}
		}
	}
}

bool UPerceptionManager::HasAnyDetectedActors() const
{
	for (const TPair<AActor*, FPerceivedActorEntry>& Entry : DetectionList)
	{
		for (const FPerceivedActorInfo& Info : Entry.Value.PerceivedActorInfos)
		{
			if (Info.bCurrentlySensed && IsValid(Info.DetectedActor))
			{
				return true; // 감지된 액터가 하나라도 있으면 true
			}
		}
	}
	return false; // 감지된 액터가 없으면 false
}


bool UPerceptionManager::GetMostRecentPerceivedInfoBySense(EAISense SearchSenseType, FPerceivedActorInfo& OutPerceivedActorInfo) const
{
	// 감지된 액터들을 순회
	for (const TPair<AActor*, FPerceivedActorEntry>& Entry : DetectionList)
	{
		const FPerceivedActorEntry& DetectionEntry = Entry.Value;

		// 각 액터에 대해 감각 정보 배열을 순회
		for (const FPerceivedActorInfo& Info : DetectionEntry.PerceivedActorInfos)
		{
			// 감각이 일치하면 최신 정보(배열의 마지막)를 반환
			if (Info.DetectedSense == SearchSenseType)
			{
				OutPerceivedActorInfo = DetectionEntry.PerceivedActorInfos.Last(); // 최신 정보
				return true;
			}
		}
	}

	// 해당 감각 정보가 없다면 false 반환
	return false;
}

bool UPerceptionManager::GetMostRecentPerceivedInfosBySense(EAISense SearchSenseType, TArray<FPerceivedActorInfo>& OutPerceivedActorInfos) const
{
	OutPerceivedActorInfos.Empty();

	// 감지된 액터들을 순회
	for (const TPair<AActor*, FPerceivedActorEntry>& Entry : DetectionList)
	{
		const FPerceivedActorEntry& DetectionEntry = Entry.Value;

		// 각 액터에 대해 감각 정보 배열을 순회
		for (const FPerceivedActorInfo& Info : DetectionEntry.PerceivedActorInfos)
		{
			// 감각이 일치하면 최신 정보(배열의 마지막)를 반환
			if (Info.DetectedSense == SearchSenseType)
			{
				OutPerceivedActorInfos.Add(Info); // 최신 정보 추가
			}
		}
	}

	// 감각 정보가 하나 이상 존재하면 true, 없으면 false 반환
	return OutPerceivedActorInfos.Num() > 0;
}
