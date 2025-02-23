// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayMessageSubsystem.h"
#include "GameplayMessageTypes2.h"

#include "AsyncAction_ListenForGameplayMessage.generated.h"

/**
 * @brief UScriptStruct, UWorld, FFrame 등의 전방 선언
 */
class UScriptStruct;
class UWorld;
struct FFrame;

/**
 * @brief 메시지 브로드캐스트 발생 시 후속 호출을 위한 'GetPayload' 함수에서,
 *        델리게이트를 트리거한 객체에 대한 참조를 얻기 위해 사용되는 프록시 객체를 나타냅니다.
 *
 * @param ActualChannel  실제로 Payload를 받은 메시지 채널입니다.
 *                       (채널은 항상 지정한 Channel로 시작하지만, partial match가 활성화된 경우 더 구체적일 수 있습니다.)
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForGameplayMessage*, ProxyObject, FGameplayTag, ActualChannel);

/**
 * @brief 비동기적으로 메시지 수신을 대기하는 액션 클래스
 *
 * 이 클래스는 GameplayMessageSubsystem을 통해 지정된 채널의 메시지를 비동기적으로 리슨합니다.
 * UCancellableAsyncAction을 상속받아, 실행 취소(cancel) 기능도 지원합니다.
 */
UCLASS(BlueprintType, meta=(HasDedicatedAsyncNode))
class GAMEPLAYMESSAGERUNTIME_API UAsyncAction_ListenForGameplayMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * @brief 지정된 채널의 메시지를 비동기적으로 수신 대기합니다.
	 *
	 * 메시지 채널, 메시지 페이로드 타입, 매칭 규칙을 지정하며, 해당 정보에 따라 메시지 수신 리스너를 등록합니다.
	 *
	 * @param WorldContextObject 월드 컨텍스트를 제공하는 객체 (예: 게임 인스턴스, 액터 등)
	 * @param Channel            메시지를 수신할 채널 태그
	 * @param PayloadType        수신할 메시지 구조체 타입 (발송자와 동일한 타입이어야 합니다)
	 * @param MatchType          메시지 매칭 규칙 (기본값: ExactMatch)
	 * @return UAsyncAction_ListenForGameplayMessage 객체 포인터 (비동기 액션 인스턴스)
	 */
	UFUNCTION(BlueprintCallable, Category = Messaging, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ListenForGameplayMessage* ListenForGameplayMessages(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EGameplayMessageMatch MatchType = EGameplayMessageMatch::ExactMatch);

	/**
	 * @brief 수신된 메시지의 페이로드를 복사하여 지정된 변수에 할당하려고 시도합니다.
	 *
	 * 전달된 OutPayload 변수의 타입은 수신된 메시지 타입과 동일해야 합니다.
	 *
	 * @param OutPayload  메시지 페이로드가 복사될 변수 (참조 전달)
	 * @return            복사가 성공하면 true, 실패하면 false를 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Messaging", meta = (CustomStructureParam = "OutPayload"))
	bool GetPayload(UPARAM(ref) int32& OutPayload);

	// Blueprint CustomThunk를 사용하기 위한 exec 함수 선언
	DECLARE_FUNCTION(execGetPayload);

	/**
	 * @brief 비동기 액션을 활성화합니다.
	 *
	 * 메시지 서브시스템에 리스너를 등록하고, 지정된 채널의 메시지를 수신하면 HandleMessageReceived 함수가 호출됩니다.
	 */
	virtual void Activate() override;

	/**
	 * @brief 비동기 액션이 종료될 준비가 되었음을 알리고, 리스너를 해제합니다.
	 */
	virtual void SetReadyToDestroy() override;

public:
	/**
	 * @brief 메시지가 지정된 채널에서 브로드캐스트될 때 호출되는 델리게이트입니다.
	 *
	 * Blueprint에서 바인딩하여 메시지 수신 시 추가 작업을 수행할 수 있습니다.
	 * 메시지 수신 후에는 GetPayload()를 통해 실제 페이로드를 가져올 수 있습니다.
	 */
	UPROPERTY(BlueprintAssignable)
	FAsyncGameplayMessageDelegate OnMessageReceived;

private:
	/**
	 * @brief 메시지 수신 시 호출되는 내부 처리 함수.
	 *
	 * 메시지 채널, 메시지 구조체 타입, 페이로드 포인터를 인자로 받아서 델리게이트를 방송합니다.
	 *
	 * @param Channel    수신된 메시지의 채널 태그
	 * @param StructType 수신된 메시지의 구조체 타입
	 * @param Payload    수신된 메시지 페이로드 포인터
	 */
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

private:
	/** 수신된 메시지 페이로드에 대한 포인터 (GetPayload에서 사용) */
	const void* ReceivedMessagePayloadPtr = nullptr;

	/** 액션이 생성된 월드를 참조하는 약한 포인터 */
	TWeakObjectPtr<UWorld> WorldPtr;

	/** 메시지 리스너를 등록할 채널 태그 */
	FGameplayTag ChannelToRegister;

	/** 메시지 페이로드의 구조체 타입 (약한 참조) */
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;

	/** 메시지 매칭 규칙 (기본값: ExactMatch) */
	EGameplayMessageMatch MessageMatchType = EGameplayMessageMatch::ExactMatch;

	/** GameplayMessageSubsystem에 등록된 리스너 핸들 */
	FGameplayMessageListenerHandle ListenerHandle;
};

