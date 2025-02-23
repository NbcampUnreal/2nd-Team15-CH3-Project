#pragma once

#include "CoreMinimal.h"
#include "EnemyAITypes.h"
#include "Perception/AIPerceptionTypes.h"
#include "UObject/Object.h"
#include "AIDectionInfoTypes.generated.h"


/**
 * 하나의 감지 정보 (하나의 감각에 해당)
 * - AI가 특정 액터를 감지한 정보를 담고 있으며, 감지된 액터, 감지된 시간, 감지된 위치 등의 정보를 관리합니다.
 */
USTRUCT(BlueprintType)
struct FPerceivedActorInfo
{
	GENERATED_BODY()

public:
	//===========================================
	// 감지 정보 관련 변수 (Detector, DetectedActor 등)
	//===========================================

	// 감지를 수행하는 AI
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	AActor* Detector = nullptr;

	// 감지된 액터
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	AActor* DetectedActor = nullptr;

	// 마지막 감지 시간
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	float LastSensedTime = 0.f;

	// 감지 중인지 여부
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	bool bCurrentlySensed = false;

	// 감지되었지만 놓친 상태인지 여부
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	bool bLostStimulus = false;

	// 마지막 감지된 위치
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	FVector LastKnownLocation = FVector::ZeroVector;

	// 감지 당시의 센서 위치
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	FVector LastSensorLocation = FVector::ZeroVector;

	// 감지된 액터가 적인지 여부
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	bool bIsHostile = false;

	// 감지된 감각 유형 (시각, 청각 등)
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	EAISense DetectedSense = EAISense::None;

	// 감지된 자극 데이터
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Info")
	FAIStimulus SenseData;

public:
	/** 감지된 자극을 업데이트하는 함수 */
	void UpdateWithStimulus(const FAIStimulus& NewStimulus, float CurrentTime, bool bHostile);


	FPerceivedActorInfo()
	{
		
	};

	FPerceivedActorInfo(const FPerceivedActorInfo& Other)
	{
		Detector = Other.Detector;
		DetectedActor = Other.DetectedActor;
		LastSensedTime = Other.LastSensedTime;
		bCurrentlySensed = Other.bCurrentlySensed;
		bLostStimulus = Other.bLostStimulus;
		LastKnownLocation = Other.LastKnownLocation;
		LastSensorLocation = Other.LastSensorLocation;
		bIsHostile = Other.bIsHostile;
		DetectedSense = Other.DetectedSense;
		SenseData = Other.SenseData;  // 복사 생성자에서 복사
	}

	FPerceivedActorInfo& operator=(const FPerceivedActorInfo& Other)
	{
		if (this != &Other)  // 자기 자신에게 대입되는 경우를 방지
		{
			Detector = Other.Detector;
			DetectedActor = Other.DetectedActor;
			LastSensedTime = Other.LastSensedTime;
			bCurrentlySensed = Other.bCurrentlySensed;
			bLostStimulus = Other.bLostStimulus;
			LastKnownLocation = Other.LastKnownLocation;
			LastSensorLocation = Other.LastSensorLocation;
			bIsHostile = Other.bIsHostile;
			DetectedSense = Other.DetectedSense;
			SenseData = Other.SenseData;
		}

		return *this;
	}


	
	// 동등성 비교 연산자 (==)
	bool operator==(const FPerceivedActorInfo& Other) const
	{
		return (Detector == Other.Detector) &&
			(DetectedActor == Other.DetectedActor) &&
			(DetectedSense == Other.DetectedSense);
	}

	// 불일치 비교 연산자 (!=)
	bool operator!=(const FPerceivedActorInfo& Other) const
	{
		return !(*this == Other); // == 연산자를 사용해서 반대로 처리
	}
};


/**
 * 감지된 액터에 대한 여러 감각 정보를 담고 있는 엔트리 구조체
 * - 각 액터는 여러 감각 정보를 가질 수 있기 때문에, 감지된 액터에 대한 감각 정보를 저장하는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FPerceivedActorEntry
{
	GENERATED_BODY()

public:
	// 감지된 액터에 대한 감각 정보 배열
	UPROPERTY(BlueprintReadWrite, Category="Perceived Actor Entry")
	TArray<FPerceivedActorInfo> PerceivedActorInfos;
};


/**
 * 여러 감지 정보(FPerceivedActorInfo)를 '스택'처럼 관리하는 매니저
 * - 여러 감지된 액터에 대한 정보를 관리하는 클래스입니다. 각 액터에 대해 여러 감각 정보를 관리할 수 있습니다.
 */
UCLASS(BlueprintType)
class SHOOTERPRO_API UPerceptionManager : public UObject
{
	GENERATED_BODY()

public:
	UPerceptionManager();

protected:
	virtual void BeginDestroy() override;

public:
	//===========================================
	// 주요 기능 관련 함수들
	//===========================================

	/** 감지 정보를 추가하거나 업데이트하는 함수 */
	void AddOrUpdateDetection(AActor* Detector, AActor* DetectedActor, EAISense SenseType, const FAIStimulus& NewStimulus, bool bHostile, float CurrentTime);

	/** 특정 액터에 대한 감지 정보를 가져오는 함수 */
	const FPerceivedActorEntry* GetDetectionEntry(AActor* Actor) const;

	/** 현재 감지 중인 액터들을 가져오는 함수 */
	void GetCurrentlySensedActors(TArray<AActor*>& OutActors) const;

	/** 감지된 모든 액터를 가져오는 함수 */
	void GetAllDetectedActors(TArray<AActor*>& OutActors) const;

	/** 특정 감각으로 감지된 액터를 가져오는 함수 */
	void GetDetectedActorsBySense(EAISense SenseType, TArray<AActor*>& OutActors) const;

	/** 특정 액터에 대한 감각별 업데이트 */
	void TickSenseDetectionsForActor(float DeltaTime, EAISense TickSense, const FAIStimulus& TickStimulus, AActor* RelevantActor);

	/** 감지 정보 만료 처리 */
	void RemoveExpiredDetectionInfos(float CurrentTime);

	/** 액터를 제거 */
	void ForgetActor(AActor* Actor);

	/** 감지 정보가 있는지 확인 */
	bool HasAnyDetectedActors() const;

	/**
	 * 특정 감각에 대해 가장 마지막에 추가된 감각 정보를 반환하는 함수
	 * 
	 * @param SearchSenseType: 검색할 감각 유형 (Sight, Hearing, Damage)
	 * @param OutPerceivedActorInfo: 가장 최근에 추가된 감각 정보를 반환
	 * @return: 감각 정보가 존재하면 true, 없으면 false
	 */
	bool GetMostRecentPerceivedInfoBySense(EAISense SearchSenseType, FPerceivedActorInfo& OutPerceivedActorInfo) const;


	/**
	 * 특정 감각에 대해 가장 최근에 추가된 감각 정보를 모두 반환하는 함수
	 * 
	 * @param SearchSenseType: 검색할 감각 유형 (Sight, Hearing, Damage)
	 * @param OutPerceivedActorInfos: 가장 최근에 추가된 감각 정보를 배열 형태로 반환
	 * @return: 감각 정보가 존재하면 true, 없으면 false
	 */
	bool GetMostRecentPerceivedInfosBySense(EAISense SearchSenseType, TArray<FPerceivedActorInfo>& OutPerceivedActorInfos) const;

private:
	/** 월드를 얻어오는 함수 (예: 액터를 기준으로 월드 정보를 얻을 때) */
	UWorld* GetWorldFromSomewhere() const;

public:
	// 각 액터에 대한 감지 정보 목록
	UPROPERTY(BlueprintReadWrite)
	TMap<AActor*, FPerceivedActorEntry> DetectionList;


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerceptionUpdatedDelegate, const FPerceivedActorInfo&, PerceivedActorInfo);

	UPROPERTY(BlueprintAssignable, Category="PerceptionManager|Delegate")
	FPerceptionUpdatedDelegate OnAddPerceptionUpdated;

	UPROPERTY(BlueprintAssignable, Category="PerceptionManager|Delegate")
	FPerceptionUpdatedDelegate OnRemoveExpiredDetection;
};
