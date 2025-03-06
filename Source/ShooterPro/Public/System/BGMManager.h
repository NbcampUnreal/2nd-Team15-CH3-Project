#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"

#include "Components/AudioComponent.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "BGMManager.generated.h"


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FBGMPayload
{
	GENERATED_BODY()

	FBGMPayload()
	{
	}
};


UCLASS()
class SHOOTERPRO_API ABGMManager : public AActor
{
	GENERATED_BODY()

public:
	ABGMManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "BGM")
	void ChangeBGM(USoundBase* NewMusic, float FadeOutTime = 1.0f, float FadeInTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void StopBGM(float FadeOutTime = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "BGM")
	void SetBGMVolume(float NewVolume, float FadeDuration = 1.0f);

	/**
	 * @brief 시퀀스를 재생하기 전에 BGM을 중단(FadeOut)하고,
	 *        시퀀스가 종료되면 DefaultBGM을 재생
	 * @param SequenceToPlay   재생할 LevelSequence
	 * @param FadeOutTime      BGM FadeOut 시간
	 * @param FadeInTimeAfter  시퀀스 끝난 후 BGM FadeIn 시간
	 */
	UFUNCTION(BlueprintCallable, Category="BGM|Sequence")
	void StopBGMAndPlaySequence(ULevelSequence* SequenceToPlay, float FadeOutTime = 1.f, float FadeInTimeAfter = 2.f);

	UFUNCTION(BlueprintCallable, Category="BGM|Sequence")
	void StopBGMAndPlaySequenceActor(
		ALevelSequenceActor* SequenceActor,
		float FadeOutTime = 1.f,
		float FadeInTimeAfter = 2.f
	);

protected:
	/** 내부 BGM 재생 함수 */
	void StartBGM(USoundBase* MusicToPlay, float FadeInTime);

	/** FadeOut이 끝난 시점에 호출되는 콜백 */
	void OnFadeOutFinished();

	/** 시퀀스 종료 콜백 - DefaultBGM 재생 후 Delegate 해제 */
	UFUNCTION()
	void OnSequenceStopped();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BGM")
	UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BGM")
	USoundBase* DefaultBGM;

	// FadeOut 후 재생할 음악
	UPROPERTY(Transient)
	USoundBase* PendingMusic;

	float PendingFadeInTime;

	FTimerHandle FadeOutTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BGM", meta=(ClampMin="0.0"))
	float DefaultVolume = 1.0f;

	// 시퀀스 종료 후 BGM FadeIn 시간
	float SequenceBGMFadeInTime = 2.f;

	// 시퀀스 플레이어 레퍼런스(Delegate RemoveDynamic 용도)
	UPROPERTY(Transient)
	ULevelSequencePlayer* CurrentSequencePlayer;
};
