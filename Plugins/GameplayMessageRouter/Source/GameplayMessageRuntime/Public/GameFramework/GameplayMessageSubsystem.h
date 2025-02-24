// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameplayMessageTypes2.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/WeakObjectPtr.h"

#include "GameplayMessageSubsystem.generated.h"

/**
 * @brief UGameplayMessageSubsystem 클래스와 관련된 로그 출력 카테고리
 * 
 * 메시지 시스템과 관련된 디버깅 및 로그 출력을 위한 카테고리입니다.
 */
class UGameplayMessageSubsystem;
struct FFrame;

// 메시지 서브시스템 관련 로그 카테고리 선언 (로그 레벨은 Log, 출력 범위는 All)
GAMEPLAYMESSAGERUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplayMessageSubsystem, Log, All);

/**
 * @brief 비동기 메시지 리스너 액션 클래스 (전방 선언)
 *
 * 메시지를 비동기 방식으로 리슨할 때 사용하는 액션 클래스입니다.
 */
class UAsyncAction_ListenForGameplayMessage;

/**
 * @brief 메시지 리스너 등록 핸들 구조체
 * 
 * 이 구조체는 메시지 리스너 등록 시 반환되는 핸들로, 등록 해제할 때 사용됩니다.
 * 핸들을 통해 해당 리스너를 언리얼 메시지 시스템에서 제거할 수 있습니다.
 */
USTRUCT(BlueprintType)
struct GAMEPLAYMESSAGERUNTIME_API FGameplayMessageListenerHandle
{
public:
	GENERATED_BODY()

	/** 기본 생성자 */
	FGameplayMessageListenerHandle()
	{
	}

	/**
	 * @brief 현재 핸들에 해당하는 리스너 등록을 해제합니다.
	 */
	void Unregister();

	/**
	 * @brief 핸들의 유효성을 확인합니다.
	 * @return ID가 0이 아니면 유효한 핸들로 간주합니다.
	 */
	bool IsValid() const { return ID != 0; }

private:
	/** 메시지 서브시스템에 대한 약한 참조 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameplayMessageSubsystem> Subsystem;

	/** 메시지 채널을 나타내는 태그 */
	UPROPERTY(Transient)
	FGameplayTag Channel;

	/** 리스너 등록 시 할당된 고유 ID (0이면 비유효) */
	UPROPERTY(Transient)
	int32 ID = 0;

	/** 내부 상태 정리(클리어) 델리게이트 핸들 (내부 사용) */
	FDelegateHandle StateClearedHandle;

	// UGameplayMessageSubsystem에서만 접근할 수 있도록 friend 선언
	friend UGameplayMessageSubsystem;

	/**
	 * @brief 생성자 (내부용)
	 * @param InSubsystem 메시지 서브시스템 포인터
	 * @param InChannel 등록된 채널 태그
	 * @param InID 할당된 리스너 ID
	 */
	FGameplayMessageListenerHandle(UGameplayMessageSubsystem* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID)
	{
	}
};

/** 
 * @brief 단일 등록 리스너 정보 구조체
 * 
 * 이 구조체는 하나의 메시지 리스너에 대한 정보를 담고 있으며,
 * 콜백 함수, 할당된 핸들 ID, 매칭 타입 등 다양한 정보를 포함합니다.
 */
USTRUCT()
struct FGameplayMessageListenerData
{
	GENERATED_BODY()

	/** 메시지 수신 시 호출되는 콜백 함수
	 *  매개변수: 실제 메시지 태그, 메시지 구조체 타입, 메시지 데이터 포인터
	 */
	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	/** 리스너의 고유 핸들 ID */
	int32 HandleID;

	/** 메시지 매칭 타입 (정확 매치, 부분 매치 등) */
	EGameplayMessageMatch MatchType;

	/** 리스너가 기대하는 메시지 구조체 타입 (약한 참조) */
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;

	/** ListenerStructType이 유효한 타입이었는지 여부 */
	bool bHadValidType = false;
};

/**
 * @brief 게임 내 메시지를 송수신하는 메시지 서브시스템 클래스
 *
 * 이 시스템은 서로 직접 알지 못하는 이벤트 발생자와 리스너 간에,
 * 동일한 메시지 형식을 사용해 통신할 수 있도록 지원합니다.
 * 게임 인스턴스 서브시스템으로서, UGameInstance::GetSubsystem<UGameplayMessageSubsystem>()로 접근할 수 있습니다.
 *
 * 호출 순서는 보장되지 않으므로, 여러 리스너가 같은 채널에 등록된 경우 순서에 의존하지 말아야 합니다.
 */
UCLASS()
class GAMEPLAYMESSAGERUNTIME_API UGameplayMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// 비동기 메시지 리스너 액션 클래스와 친구 선언 (내부 접근 허용)
	friend UAsyncAction_ListenForGameplayMessage;

public:
	/**
	 * @brief 지정된 WorldContextObject가 속한 게임 인스턴스의 메시지 서브시스템을 반환합니다.
	 * @param WorldContextObject 월드 컨텍스트 객체 (예: 캐릭터, 액터 등)
	 * @return UGameplayMessageSubsystem 참조
	 */
	static UGameplayMessageSubsystem& Get(const UObject* WorldContextObject);

	/**
	 * @brief 지정된 WorldContextObject에 유효한 메시지 서브시스템 인스턴스가 존재하는지 확인합니다.
	 * @param WorldContextObject 월드 컨텍스트 객체
	 * @return 인스턴스가 있으면 true, 없으면 false
	 */
	static bool HasInstance(const UObject* WorldContextObject);

	//~ USubsystem 인터페이스
	virtual void Deinitialize() override;
	//~ End of USubsystem 인터페이스

	/**
	 * @brief 지정된 채널로 메시지를 방송합니다.
	 *
	 * @tparam FMessageStructType 메시지 구조체 타입 (UScriptStruct이어야 함)
	 * @param Channel 메시지 채널 태그
	 * @param Message 전송할 메시지 (구조체 타입)
	 */
	template <typename FMessageStructType>
	void BroadcastMessage(FGameplayTag Channel, const FMessageStructType& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/**
	 * @brief 지정된 채널에서 메시지를 수신하도록 리스너를 등록합니다.
	 *
	 * 콜백 함수는 메시지를 수신할 때 호출되며, 반드시 보내는 쪽과 같은 UScriptStruct 타입을 사용해야 합니다.
	 *
	 * @tparam FMessageStructType 메시지 구조체 타입
	 * @param Channel 메시지 채널 태그
	 * @param Callback 메시지 수신 시 실행할 콜백 함수 (콜백 인자는 채널 태그, 메시지 구조체)
	 * @param MatchType 메시지 매칭 타입 (기본값은 ExactMatch)
	 * @return 등록된 리스너 핸들 (나중에 Unregister 시 사용)
	 */
	template <typename FMessageStructType>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TFunction<void(FGameplayTag, const FMessageStructType&)>&& Callback, EGameplayMessageMatch MatchType = EGameplayMessageMatch::ExactMatch)
	{
		// 내부 콜백 함수로 래핑 (void* 메시지 데이터를 FMessageStructType으로 캐스팅)
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
		};

		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		return RegisterListenerInternal(Channel, ThunkCallback, StructType, MatchType);
	}

	/**
	 * @brief 지정된 채널에서 메시지를 수신하도록, 멤버 함수를 통한 리스너를 등록합니다.
	 *
	 * 객체의 유효성을 약하게 검사하여, 객체가 유효할 때만 콜백을 실행합니다.
	 *
	 * @tparam FMessageStructType 메시지 구조체 타입
	 * @tparam TOwner 등록할 객체의 타입 (기본은 UObject)
	 * @param Channel 메시지 채널 태그
	 * @param Object 콜백을 호출할 객체 인스턴스
	 * @param Function 호출할 멤버 함수 (인자는 채널 태그, 메시지 구조체)
	 * @return 등록된 리스너 핸들
	 */
	template <typename FMessageStructType, typename TOwner = UObject>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return RegisterListener<FMessageStructType>(Channel,
		                                            [WeakObject, Function](FGameplayTag Channel, const FMessageStructType& Payload)
		                                            {
			                                            if (TOwner* StrongObject = WeakObject.Get())
			                                            {
				                                            (StrongObject->*Function)(Channel, Payload);
			                                            }
		                                            });
	}

	/**
	 * @brief 고급 옵션을 지원하는 파라미터를 사용하여 지정된 채널의 메시지를 수신하도록 리스너를 등록합니다.
	 *
	 * @tparam FMessageStructType 메시지 구조체 타입
	 * @param Channel 메시지 채널 태그
	 * @param Params 고급 리스너 등록을 위한 파라미터 구조체
	 * @return 등록된 리스너 핸들
	 */
	template <typename FMessageStructType>
	FGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, FGameplayMessageListenerParams<FMessageStructType>& Params)
	{
		FGameplayMessageListenerHandle Handle;

		// 메시지 수신 콜백이 정의되어 있다면, 내부 콜백 함수로 래핑하여 등록
		if (Params.OnMessageReceivedCallback)
		{
			auto ThunkCallback = [InnerCallback = Params.OnMessageReceivedCallback](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
			{
				InnerCallback(ActualTag, *reinterpret_cast<const FMessageStructType*>(SenderPayload));
			};

			const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
			Handle = RegisterListenerInternal(Channel, ThunkCallback, StructType, Params.MatchType);
		}

		return Handle;
	}

	/**
	 * @brief 이전에 RegisterListener로 등록된 메시지 리스너를 제거합니다.
	 *
	 * @param Handle 등록 시 반환된 핸들
	 */
	void UnregisterListener(FGameplayMessageListenerHandle Handle);

protected:
	/**
	 * @brief 내부적으로 메시지를 방송하는 함수
	 *
	 * 실제 메시지를 방송할 때 사용되며, 전달된 UScriptStruct 타입과 메시지 데이터를 기반으로 호출합니다.
	 *
	 * @param Channel 메시지 채널 태그
	 * @param StructType 메시지 구조체 타입
	 * @param MessageBytes 메시지 데이터의 포인터
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category=Messaging, meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="Broadcast Message"))
	void K2_BroadcastMessage(FGameplayTag Channel, const int32& Message);

	// Blueprint의 CustomThunk 기능에 의해 호출되는 함수 선언
	DECLARE_FUNCTION(execK2_BroadcastMessage);

private:
	/**
	 * @brief BroadcastMessage의 내부 헬퍼 함수
	 *
	 * @param Channel 메시지 채널 태그
	 * @param StructType 메시지 구조체 타입
	 * @param MessageBytes 메시지 데이터의 포인터
	 */
	void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

	/**
	 * @brief 리스너 등록의 내부 헬퍼 함수
	 *
	 * @param Channel 메시지 채널 태그
	 * @param Callback 메시지 수신 시 호출할 내부 콜백 함수
	 * @param StructType 메시지 구조체 타입
	 * @param MatchType 메시지 매칭 타입
	 * @return 등록된 리스너 핸들
	 */
	FGameplayMessageListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType,
		EGameplayMessageMatch MatchType);

	/**
	 * @brief 내부적으로 리스너를 제거하는 함수
	 *
	 * @param Channel 메시지 채널 태그
	 * @param HandleID 제거할 리스너의 ID
	 */
	void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

private:
	/** 지정된 채널의 모든 리스너 목록과 현재 할당된 핸들 ID를 저장하는 구조체 */
	struct FChannelListenerList
	{
		/** 채널에 등록된 모든 리스너 데이터 배열 */
		TArray<FGameplayMessageListenerData> Listeners;

		/** 마지막 할당된 핸들 ID (새 리스너 등록 시 증가) */
		int32 HandleID = 0;
	};

private:
	/** 각 메시지 채널별 리스너 목록을 저장하는 맵 */
	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};
