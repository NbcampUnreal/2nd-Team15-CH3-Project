// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

// 인라인 생성된 CPP 파일 포함 (Unreal Engine 내부 매크로)
#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayMessageSubsystem)

// 메시지 서브시스템 로그 카테고리 정의
DEFINE_LOG_CATEGORY(LogGameplayMessageSubsystem);

namespace UE
{
	namespace GameplayMessageSubsystem
	{
		// 콘솔 변수: 메시지 방송 시 로그 출력을 할지 여부를 결정 (0이면 비활성, 1 이상이면 활성)
		static int32 ShouldLogMessages = 0;
		static FAutoConsoleVariableRef CVarShouldLogMessages(TEXT("GameplayMessageSubsystem.LogMessages"),
		                                                     ShouldLogMessages,
		                                                     TEXT("GameplayMessageSubsystem을 통해 방송되는 메시지를 로그에 출력할지 여부를 결정합니다."));
	}
}

//////////////////////////////////////////////////////////////////////
// FGameplayMessageListenerHandle

/**
 * @brief FGameplayMessageListenerHandle의 Unregister() 함수 구현
 * 
 * 현재 핸들에 등록된 리스너를 메시지 서브시스템에서 제거하고, 핸들을 초기화합니다.
 */
void FGameplayMessageListenerHandle::Unregister()
{
	// 서브시스템이 유효하면, 해당 서브시스템의 UnregisterListener 함수를 호출합니다.
	if (UGameplayMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		// 서브시스템 참조 초기화 및 채널, ID 초기화
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// UGameplayMessageSubsystem

/**
 * @brief UGameplayMessageSubsystem의 Get 함수 구현
 *
 * 지정된 WorldContextObject의 게임 인스턴스에서 메시지 서브시스템을 검색하여 반환합니다.
 *
 * @param WorldContextObject 월드 컨텍스트 객체
 * @return 메시지 서브시스템에 대한 참조
 */
UGameplayMessageSubsystem& UGameplayMessageSubsystem::Get(const UObject* WorldContextObject)
{
	// WorldContextObject로부터 UWorld를 얻어옴. 에러 발생 시 Assert
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);

	// 게임 인스턴스에서 UGameplayMessageSubsystem 서브시스템을 검색
	UGameInstance* WorldGameInstance = World->GetGameInstance();
	UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(WorldGameInstance);
	check(Router);
	return *Router;
}

/**
 * @brief UGameplayMessageSubsystem의 HasInstance 함수 구현
 *
 * 지정된 WorldContextObject에 유효한 메시지 서브시스템 인스턴스가 존재하는지 확인합니다.
 *
 * @param WorldContextObject 월드 컨텍스트 객체
 * @return 인스턴스가 존재하면 true, 아니면 false
 */
bool UGameplayMessageSubsystem::HasInstance(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UGameplayMessageSubsystem* Router = World != nullptr ? UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance()) : nullptr;
	return Router != nullptr;
}

/**
 * @brief UGameplayMessageSubsystem의 Deinitialize 함수 구현
 *
 * 서브시스템이 해제될 때, 리스너 맵을 초기화하고 상위 클래스의 Deinitialize 함수를 호출합니다.
 */
void UGameplayMessageSubsystem::Deinitialize()
{
	// 모든 리스너 데이터를 제거하여 리셋
	ListenerMap.Reset();

	// 상위 클래스의 Deinitialize 호출
	Super::Deinitialize();
}

/**
 * @brief 내부적으로 메시지를 방송하는 함수 구현
 *
 * 지정된 채널과 메시지 구조체 타입, 메시지 데이터를 기반으로 모든 등록된 리스너에게 메시지를 방송합니다.
 *
 * @param Channel 메시지 채널 태그
 * @param StructType 메시지 구조체 타입
 * @param MessageBytes 메시지 데이터의 포인터
 */
void UGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes)
{
	// 로그 출력 설정이 활성화되어 있으면 메시지 정보를 로그에 기록
	if (UE::GameplayMessageSubsystem::ShouldLogMessages != 0)
	{
		FString* pContextString = nullptr;
#if WITH_EDITOR
		// 에디터 모드일 경우, PlayInEditor 컨텍스트 문자열 사용
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			pContextString = &GPlayInEditorContextString;
		}
#endif
		// 메시지 데이터를 사람이 읽을 수 있는 문자열로 변환
		FString HumanReadableMessage;
		StructType->ExportText(/*out*/ HumanReadableMessage, MessageBytes, /*Defaults=*/ nullptr, /*OwnerObject=*/ nullptr, PPF_None, /*ExportRootScope=*/ nullptr);
		// 로그 출력 (메시지 방송 시의 상세 정보 기록)
		UE_LOG(LogGameplayMessageSubsystem, Log, TEXT("메시지 방송: %s, 채널: %s, 메시지: %s"), pContextString ? **pContextString : *GetPathNameSafe(this), *Channel.ToString(), *HumanReadableMessage);
	}

	// 메시지 방송을 위한 루프: 채널의 직접 태그 및 상위 태그에 대해 순차적으로 리스너에게 전달
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		// 현재 태그에 등록된 리스너 목록 검색
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			// 콜백 실행 도중 리스너 목록이 변경될 수 있으므로, 배열을 복사하여 사용
			TArray<FGameplayMessageListenerData> ListenerArray(pList->Listeners);

			// 복사된 리스너 배열을 순회하며 메시지 전달
			for (const FGameplayMessageListenerData& Listener : ListenerArray)
			{
				// 초기 태그이거나, 부분 매치인 경우에만 실행
				if (bOnInitialTag || (Listener.MatchType == EGameplayMessageMatch::PartialMatch))
				{
					// 리스너 타입이 한때 유효했으나 현재 유효하지 않다면, 로그 경고 후 리스너 제거
					if (Listener.bHadValidType && !Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("채널 %s의 리스너 구조체 타입이 유효하지 않습니다. 리스너를 제거합니다."), *Channel.ToString());
						UnregisterListenerInternal(Channel, Listener.HandleID);
						continue;
					}

					// 전달되는 메시지 구조체 타입이 리스너가 기대하는 타입과 맞는지 확인
					if (!Listener.bHadValidType || StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						// 콜백 함수 실행하여 메시지를 전달
						Listener.ReceivedCallback(Channel, StructType, MessageBytes);
					}
					else
					{
						// 구조체 타입이 불일치할 경우 에러 로그 출력
						UE_LOG(LogGameplayMessageSubsystem, Error, TEXT("채널 %s에서 구조체 타입 불일치 (송신 타입: %s, 리스너(%s)가 기대하는 타입: %s)"),
						       *Channel.ToString(),
						       *StructType->GetPathName(),
						       *Tag.ToString(),
						       *Listener.ListenerStructType->GetPathName());
					}
				}
			}
		}
		// 초기 태그 처리 후에는 false로 설정
		bOnInitialTag = false;
	}
}

/**
 * @brief Blueprint를 통한 메시지 방송 함수 (K2_BroadcastMessage)
 *
 * 이 함수는 직접 호출되지 않으며, execK2_BroadcastMessage 함수에서 실제 처리됩니다.
 *
 * @param Channel 메시지 채널 태그
 * @param Message 메시지 데이터 (정수형)
 */
void UGameplayMessageSubsystem::K2_BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	// 이 함수는 절대로 호출되지 않으며, execK2_BroadcastMessage에서 처리됨.
	checkNoEntry();
}

/**
 * @brief Blueprint CustomThunk를 위한 execK2_BroadcastMessage 함수 구현
 *
 * 스택에서 메시지 데이터를 추출하여 BroadcastMessageInternal 함수를 호출합니다.
 */
DEFINE_FUNCTION(UGameplayMessageSubsystem::execK2_BroadcastMessage)
{
	// 채널 태그 추출
	P_GET_STRUCT(FGameplayTag, Channel);

	// 스택에서 메시지 구조체 데이터 추출
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	// 메시지 데이터가 올바른지 검증 후, BroadcastMessageInternal 호출
	if (ensure((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr)))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct, MessagePtr);
	}
}

/**
 * @brief RegisterListenerInternal의 내부 구현 함수
 *
 * 지정된 채널에 리스너를 등록하고, 고유 핸들 ID를 할당합니다.
 *
 * @param Channel 메시지 채널 태그
 * @param Callback 메시지 수신 시 호출할 내부 콜백 함수
 * @param StructType 리스너가 기대하는 메시지 구조체 타입
 * @param MatchType 메시지 매칭 타입 (정확 또는 부분 매치)
 * @return 등록된 리스너 핸들
 */
FGameplayMessageListenerHandle UGameplayMessageSubsystem::RegisterListenerInternal(FGameplayTag Channel, TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback, const UScriptStruct* StructType, EGameplayMessageMatch MatchType)
{
	// 지정된 채널에 대한 리스너 목록을 가져오거나 새로 생성
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	// 새로운 리스너 데이터를 배열에 추가하고, 내부 핸들 ID 할당
	FGameplayMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.bHadValidType = StructType != nullptr;
	Entry.HandleID = ++List.HandleID;
	Entry.MatchType = MatchType;

	// 등록된 리스너 핸들을 반환
	return FGameplayMessageListenerHandle(this, Channel, Entry.HandleID);
}

/**
 * @brief 등록된 리스너를 핸들을 통해 제거하는 함수 구현
 *
 * 핸들이 유효한 경우, 내부 UnregisterListenerInternal 함수를 호출하여 해당 리스너를 제거합니다.
 *
 * @param Handle 등록 시 반환된 리스너 핸들
 */
void UGameplayMessageSubsystem::UnregisterListener(FGameplayMessageListenerHandle Handle)
{
	if (Handle.IsValid())
	{
		// 핸들의 서브시스템이 현재 객체(this)와 동일한지 확인
		check(Handle.Subsystem == this);

		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogGameplayMessageSubsystem, Warning, TEXT("유효하지 않은 핸들로 리스너 제거 시도"));
	}
}

/**
 * @brief 내부적으로 리스너를 제거하는 함수 구현
 *
 * 지정된 채널에서 해당 HandleID에 해당하는 리스너를 제거하며, 리스너가 없으면 채널 자체를 맵에서 제거합니다.
 *
 * @param Channel 메시지 채널 태그
 * @param HandleID 제거할 리스너의 고유 ID
 */
void UGameplayMessageSubsystem::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	if (FChannelListenerList* pList = ListenerMap.Find(Channel))
	{
		// HandleID에 해당하는 리스너의 인덱스를 검색
		int32 MatchIndex = pList->Listeners.IndexOfByPredicate([ID = HandleID](const FGameplayMessageListenerData& Other) { return Other.HandleID == ID; });
		if (MatchIndex != INDEX_NONE)
		{
			// 인덱스에 해당하는 리스너 제거 (배열의 순서를 보장하지 않으므로 Swap 제거)
			pList->Listeners.RemoveAtSwap(MatchIndex);
		}

		// 해당 채널에 등록된 리스너가 없으면, 채널 항목 자체를 제거
		if (pList->Listeners.Num() == 0)
		{
			ListenerMap.Remove(Channel);
		}
	}
}
