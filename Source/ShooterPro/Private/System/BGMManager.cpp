#include "System/BGMManager.h"

#include "LevelSequenceActor.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "ProGmaeplayTag.h"
#include "Components/AudioComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"


ABGMManager::ABGMManager()
{
    PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;
    AudioComponent->bAutoDestroy = false;

	// 기본 볼륨
	DefaultVolume = 1.0f;
    SequenceBGMFadeInTime = 2.f;
    CurrentSequencePlayer = nullptr;
}

void ABGMManager::BeginPlay()
{
	Super::BeginPlay();

    // 원래 코드대로, DefaultBGM이 있으면 자동 재생
	if (DefaultBGM)
	{
		ChangeBGM(DefaultBGM, 0.f, 2.f);
	}
}

void ABGMManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AudioComponent)
	{
		AudioComponent->Stop();
	}

    // 혹시 시퀀스 플레이어가 살아있다면 Delegate 해제
    if (CurrentSequencePlayer)
    {
        CurrentSequencePlayer->OnStop.RemoveDynamic(this, &ABGMManager::OnSequenceStopped);
        CurrentSequencePlayer = nullptr;
    }

	Super::EndPlay(EndPlayReason);
}

void ABGMManager::ChangeBGM(USoundBase* NewMusic, float FadeOutTime, float FadeInTime)
{
    if (!AudioComponent) return;

	if (!NewMusic)
	{
		StopBGM(FadeOutTime);
		return;
	}

	if (AudioComponent->IsPlaying())
	{
		PendingMusic = NewMusic;
		PendingFadeInTime = FadeInTime;

		GetWorldTimerManager().ClearTimer(FadeOutTimerHandle);
		AudioComponent->FadeOut(FadeOutTime, 0.f);

		GetWorldTimerManager().SetTimer(
			FadeOutTimerHandle,
			this,
			&ABGMManager::OnFadeOutFinished,
			FadeOutTime,
			false
		);
	}
	else
	{
		PendingMusic = nullptr;
		StartBGM(NewMusic, FadeInTime);
	}
}

void ABGMManager::StopBGM(float FadeOutTime)
{
	if (!AudioComponent) return;

	if (AudioComponent->IsPlaying())
	{
		PendingMusic = nullptr;
		PendingFadeInTime = 0.f;

		GetWorldTimerManager().ClearTimer(FadeOutTimerHandle);
		AudioComponent->FadeOut(FadeOutTime, 0.f);

		GetWorldTimerManager().SetTimer(
			FadeOutTimerHandle,
			this,
			&ABGMManager::OnFadeOutFinished,
			FadeOutTime,
			false
		);
	}
	else
	{
		AudioComponent->Stop();
	}
}

void ABGMManager::SetBGMVolume(float NewVolume, float FadeDuration)
{
    if (!AudioComponent) return;

    AudioComponent->AdjustVolume(FadeDuration, NewVolume);
    UE_LOG(LogTemp, Log, TEXT("BGM volume change -> %f over %f seconds"), NewVolume, FadeDuration);
}

void ABGMManager::OnFadeOutFinished()
{
	AudioComponent->Stop();

	if (PendingMusic)
	{
		USoundBase* MusicToPlay = PendingMusic;
		float FadeInTime = PendingFadeInTime;

		PendingMusic = nullptr;
		PendingFadeInTime = 0.f;

		StartBGM(MusicToPlay, FadeInTime);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("BGM fully stopped."));
	}
}

void ABGMManager::StartBGM(USoundBase* MusicToPlay, float FadeInTime)
{
	if (!AudioComponent || !MusicToPlay) return;

	AudioComponent->SetSound(MusicToPlay);
	AudioComponent->FadeIn(FadeInTime, DefaultVolume);

	UE_LOG(LogTemp, Log, TEXT("BGM started: %s"), *MusicToPlay->GetName());
}

// -------------------- 시퀀스 재생 --------------------

void ABGMManager::StopBGMAndPlaySequence(ULevelSequence* SequenceToPlay, float FadeOutTime, float FadeInTimeAfter)
{
    if (!SequenceToPlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("StopBGMAndPlaySequence: No SequenceToPlay!"));
        return;
    }

    // 먼저 BGM FadeOut
    StopBGM(FadeOutTime);

    // LevelSequencePlayer 생성
    FMovieSceneSequencePlaybackSettings PlaybackSettings;
    ALevelSequenceActor* OutActor = nullptr;

    ULevelSequencePlayer* NewPlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
        GetWorld(),
        SequenceToPlay,
        PlaybackSettings,
        OutActor
    );
    if (!NewPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create LevelSequencePlayer!"));
        return;
    }

    // 기존 Player의 Delegate가 있을 수 있으므로 정리
    if (CurrentSequencePlayer)
    {
        CurrentSequencePlayer->OnStop.RemoveDynamic(this, &ABGMManager::OnSequenceStopped);
        CurrentSequencePlayer = nullptr;
    }

    // 새 Player 바인딩
    CurrentSequencePlayer = NewPlayer;
    CurrentSequencePlayer->OnStop.AddDynamic(this, &ABGMManager::OnSequenceStopped);

    // 시퀀스 끝난 후 BGM FadeIn 시간 저장
    SequenceBGMFadeInTime = FadeInTimeAfter;

    // 시퀀스 재생
    CurrentSequencePlayer->Play();
}

void ABGMManager::StopBGMAndPlaySequenceActor(ALevelSequenceActor* SequenceActor, float FadeOutTime, float FadeInTimeAfter)
{
	if (!SequenceActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("No SequenceActor!"));
		return;
	}

	// 1) BGM 중단
	StopBGM(FadeOutTime);

	// 2) 이미 시퀀스 플레이어가 Actor 안에 있을 것이므로 참조
	ULevelSequencePlayer* Player = SequenceActor->SequencePlayer; 
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("SequenceActor has no SequencePlayer!"));
		return;
	}

	// 기존 Delegate 해제
	if (CurrentSequencePlayer)
	{
		CurrentSequencePlayer->OnStop.RemoveDynamic(this, &ABGMManager::OnSequenceStopped);
		CurrentSequencePlayer = nullptr;
	}

	// 새 Player에 Delegate 등록
	CurrentSequencePlayer = Player;
	CurrentSequencePlayer->OnStop.AddDynamic(this, &ABGMManager::OnSequenceStopped);

	// 시퀀스 끝난 후 BGM FadeIn 시간 보관
	SequenceBGMFadeInTime = FadeOutTime;

	// 3) 플레이
	CurrentSequencePlayer->Play();
}

void ABGMManager::OnSequenceStopped()
{
    UE_LOG(LogTemp, Log, TEXT("Sequence finished -> Resuming DefaultBGM"));

    // Delegate 해제
    if (CurrentSequencePlayer)
    {
        CurrentSequencePlayer->OnStop.RemoveDynamic(this, &ABGMManager::OnSequenceStopped);
        CurrentSequencePlayer = nullptr;
    }

    // 시퀀스 끝났으므로, Default BGM 다시 재생
    ChangeBGM(DefaultBGM, 0.f, SequenceBGMFadeInTime);
	FBGMPayload Payload;
	
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MsgSubsystem.BroadcastMessage<FBGMPayload>(ProGameplayTags::Event_System_CreateWidget, Payload);
}