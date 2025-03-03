// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFramework/AsyncAction_ListenForGameplayMessage.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/ScriptMacros.h"
#include "UObject/Stack.h"

// 인라인 생성된 CPP 파일 포함 (Unreal Engine 내부 매크로)
#include UE_INLINE_GENERATED_CPP_BY_NAME(AsyncAction_ListenForGameplayMessage)

/**
 * @brief ListenForGameplayMessages 정적 함수 구현
 *
 * 지정된 WorldContextObject로부터 UWorld를 얻어, 메시지 서브시스템에 리스너를 등록한 후,
 * 비동기 액션 객체를 생성하여 반환합니다.
 *
 * @param WorldContextObject 월드 컨텍스트 객체 (예: 액터)
 * @param Channel            메시지를 수신할 채널 태그
 * @param PayloadType        수신할 메시지의 구조체 타입 (발송자와 동일해야 함)
 * @param MatchType          메시지 매칭 규칙 (기본값: ExactMatch)
 * @return                   생성된 UAsyncAction_ListenForGameplayMessage 객체 포인터
 */
UAsyncAction_ListenForGameplayMessage* UAsyncAction_ListenForGameplayMessage::ListenForGameplayMessages(UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType, EGameplayMessageMatch MatchType)
{
	// WorldContextObject로부터 UWorld를 얻어옴; 실패 시 nullptr 반환
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// 비동기 액션 객체 생성 및 초기화
	UAsyncAction_ListenForGameplayMessage* Action = NewObject<UAsyncAction_ListenForGameplayMessage>();
	Action->WorldPtr = World;
	Action->ChannelToRegister = Channel;
	Action->MessageStructType = PayloadType;
	Action->MessageMatchType = MatchType;
	// 게임 인스턴스에 등록하여 액션의 생명주기를 관리
	Action->RegisterWithGameInstance(World);

	return Action;
}

/**
 * @brief Activate 함수 구현
 *
 * 액션 활성화 시, 현재 월드가 유효한지 확인하고,
 * UGameplayMessageSubsystem의 리스너 등록 함수를 호출하여 메시지 리스너를 등록합니다.
 * 등록에 성공하면, 지정된 채널의 메시지 수신 시 HandleMessageReceived 함수가 호출됩니다.
 * 만약 월드가 유효하지 않거나 서브시스템 인스턴스가 없으면, 액션을 종료합니다.
 */
void UAsyncAction_ListenForGameplayMessage::Activate()
{
	if (UWorld* World = WorldPtr.Get())
	{
		// 월드 내에 GameplayMessageSubsystem 인스턴스가 존재하는지 확인
		if (UGameplayMessageSubsystem::HasInstance(World))
		{
			UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(World);

			// 현재 액션 객체에 대한 약한 참조 생성 (람다 캡쳐용)
			TWeakObjectPtr<UAsyncAction_ListenForGameplayMessage> WeakThis(this);
			// 지정된 채널에 대해 리스너 등록; 메시지 수신 시 HandleMessageReceived 호출
			ListenerHandle = Router.RegisterListenerInternal(ChannelToRegister,
			                                                 [WeakThis](FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
			                                                 {
				                                                 if (UAsyncAction_ListenForGameplayMessage* StrongThis = WeakThis.Get())
				                                                 {
					                                                 StrongThis->HandleMessageReceived(Channel, StructType, Payload);
				                                                 }
			                                                 },
			                                                 MessageStructType.Get(),
			                                                 MessageMatchType);

			return;
		}
	}

	// 월드가 유효하지 않거나 서브시스템 인스턴스가 없는 경우, 액션 종료 준비
	SetReadyToDestroy();
}

/**
 * @brief SetReadyToDestroy 함수 구현
 *
 * 액션 종료 시, 등록된 메시지 리스너를 해제하고 상위 클래스의 SetReadyToDestroy()를 호출합니다.
 */
void UAsyncAction_ListenForGameplayMessage::SetReadyToDestroy()
{
	// 리스너 해제: 더 이상 메시지 수신을 하지 않도록 함
	ListenerHandle.Unregister();

	// 상위 클래스의 SetReadyToDestroy() 호출
	Super::SetReadyToDestroy();
}

/**
 * @brief GetPayload 함수 구현 (C++ 직접 호출되지 않음)
 *
 * 이 함수는 Blueprint에서 CustomThunk를 통해 호출되며, 실제 payload 복사 작업은 execGetPayload에서 처리됩니다.
 *
 * @param OutPayload 메시지 페이로드를 복사할 변수 (참조)
 * @return            항상 false를 반환하며, checkNoEntry()에 의해 직접 호출되지 않음을 보장합니다.
 */
bool UAsyncAction_ListenForGameplayMessage::GetPayload(int32& OutPayload)
{
	// 이 함수는 직접 호출되면 안 됨을 보장
	checkNoEntry();
	return false;
}

/**
 * @brief execGetPayload 함수 구현 (Blueprint CustomThunk)
 *
 * 스택에서 전달된 매개변수를 이용해 메시지 페이로드를 복사합니다.
 * 전달된 구조체 타입이 수신된 메시지의 타입과 일치하고, 수신된 페이로드가 유효하면 복사 후 true 반환,
 * 그렇지 않으면 false를 반환합니다.
 */
DEFINE_FUNCTION(UAsyncAction_ListenForGameplayMessage::execGetPayload)
{
	// 스택에서 OutPayload에 해당하는 변수의 주소를 추출
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	// 구조체 프로퍼티로 캐스팅
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH;

	bool bSuccess = false;

	// Blueprint 노드에서 가져오려는 타입과 수신된 메시지 타입이 동일하고, 메시지 페이로드가 유효한 경우 복사 수행
	if ((StructProp != nullptr) && (StructProp->Struct != nullptr) && (MessagePtr != nullptr) && (StructProp->Struct == P_THIS->MessageStructType.Get()) && (P_THIS->ReceivedMessagePayloadPtr != nullptr))
	{
		// 메시지 구조체 복사: Source는 ReceivedMessagePayloadPtr, Destination은 MessagePtr
		StructProp->Struct->CopyScriptStruct(MessagePtr, P_THIS->ReceivedMessagePayloadPtr);
		bSuccess = true;
	}

	// 결과를 Blueprint 노드의 리턴 값에 할당
	*(bool*)RESULT_PARAM = bSuccess;
}

/**
 * @brief 메시지 수신 시 호출되는 내부 처리 함수 구현
 *
 * 수신된 메시지의 타입이 기대한 타입과 일치하면, 페이로드를 저장하고 OnMessageReceived 델리게이트를 방송합니다.
 * 만약 델리게이트가 바인딩되어 있지 않으면, 액션을 종료 처리합니다.
 *
 * @param Channel    수신된 메시지의 채널 태그
 * @param StructType 수신된 메시지의 구조체 타입
 * @param Payload    수신된 메시지 페이로드 포인터
 */
void UAsyncAction_ListenForGameplayMessage::HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload)
{
	// 메시지 타입이 기대한 타입이거나 MessageStructType이 없는 경우에만 처리
	if (!MessageStructType.Get() || (MessageStructType.Get() == StructType))
	{
		// 메시지 페이로드를 임시 저장
		ReceivedMessagePayloadPtr = Payload;

		// OnMessageReceived 델리게이트를 방송하여 수신 사실을 알림
		OnMessageReceived.Broadcast(this, Channel);

		// 페이로드 처리가 끝난 후, 포인터 초기화
		ReceivedMessagePayloadPtr = nullptr;
	}

	// 델리게이트가 바인딩되어 있지 않은 경우, 액션 종료 처리
	if (!OnMessageReceived.IsBound())
	{
		// BP 객체가 파괴된 경우, 델리게이트 바인딩이 해제되므로 안전하게 액션을 종료합니다.
		// (추후 더 적극적인 정리 메커니즘 도입 예정 - FORT-340994)
		SetReadyToDestroy();
	}
}
