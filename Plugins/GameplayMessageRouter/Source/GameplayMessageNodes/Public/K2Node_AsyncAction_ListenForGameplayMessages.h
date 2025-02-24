// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "K2Node_AsyncAction.h"

#include "K2Node_AsyncAction_ListenForGameplayMessages.generated.h"

/**
 * 전방 선언
 */
class FBlueprintActionDatabaseRegistrar;
class FKismetCompilerContext;
class FMulticastDelegateProperty;
class FString;
class UEdGraph;
class UEdGraphPin;
class UObject;

/**
 * @brief Blueprint에서 UAsyncAction_RegisterGameplayMessageReceiver의 비동기 로직을 처리하기 위해 생성되는 노드 클래스.
 *
 * 이 노드는 메시지 수신 비동기 액션의 실행 흐름을 처리하며, 
 * 페이로드 복사 및 채널 정보를 출력하는 등 내부적으로 다양한 연결 작업을 수행합니다.
 */
UCLASS()
class UK2Node_AsyncAction_ListenForGameplayMessages : public UK2Node_AsyncAction
{
	GENERATED_BODY()

	//~ UEdGraphNode 인터페이스

	/**
	 * @brief 노드가 재구성될 때 호출되는 함수.
	 *        노드 재구성 후 출력 페이로드 타입을 새로 고칩니다.
	 */
	virtual void PostReconstructNode() override;

	/**
	 * @brief 노드의 핀 기본값이 변경될 때 호출되는 함수.
	 *        PayloadType 핀의 기본값 변경 시 출력 페이로드 타입을 갱신합니다.
	 */
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;

	/**
	 * @brief 핀 위에 마우스가 올려졌을 때 보여줄 툴팁 텍스트를 반환합니다.
	 * @param Pin          대상 핀
	 * @param HoverTextOut 출력될 툴팁 문자열
	 */
	virtual void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~ End of UEdGraphNode 인터페이스
	
	//~ UK2Node 인터페이스

	/**
	 * @brief Blueprint 액션 메뉴에 노드 항목을 등록하는 함수.
	 * @param ActionRegistrar  액션 데이터베이스 등록자
	 */
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	/**
	 * @brief 노드 기본 핀들을 생성하는 함수.
	 */
	virtual void AllocateDefaultPins() override;
	//~ End of UK2Node 인터페이스

protected:
	/**
	 * @brief Delegate(델리게이트) 처리 로직을 구현하는 함수.
	 *
	 * VariableOutputs에는 프록시 객체, 페이로드, 실제 채널 변수에 대한 출력 핀이 포함됩니다.
	 * 이 함수는 내부적으로 GetPayload 플로우를 처리하며, Delegate 처리 체인의 마지막 핀을 업데이트합니다.
	 *
	 * @param VariableOutputs           출력 핀 및 로컬 변수 배열 (프록시, 페이로드, 채널)
	 * @param ProxyObjectPin            델리게이트 프록시 객체 핀
	 * @param InOutLastThenPin          실행 순서 체인에서 마지막 Then 핀 (입출력)
	 * @param SourceGraph               소스 에디터 그래프
	 * @param CompilerContext           Kismet 컴파일러 컨텍스트
	 * @return true이면 에러 없이 처리된 것이고, false이면 오류가 발생함.
	 */
	virtual bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext) override;

private:
	/**
	 * @brief 델리게이트 처리 체인의 마지막에 GetPayload 플로우를 추가하는 함수.
	 *
	 * 현재 델리게이트 프로퍼티(CurrentProperty)와 연결된 프록시, 페이로드, 실제 채널 변수들을 이용해,
	 * GetPayload 함수 호출 플로우를 생성합니다.
	 *
	 * @param CurrentProperty            현재 델리게이트 프로퍼티 (FMulticastDelegateProperty)
	 * @param ProxyObjectVar             프록시 객체 변수 관련 출력 핀 및 로컬 변수 정보
	 * @param PayloadVar                 페이로드 변수 관련 출력 핀 및 로컬 변수 정보
	 * @param ActualChannelVar           실제 채널 변수 관련 출력 핀 및 로컬 변수 정보
	 * @param InOutLastActivatedThenPin  실행 순서 체인에서 마지막 Then 핀 (입출력)
	 * @param SourceGraph                소스 에디터 그래프
	 * @param CompilerContext            Kismet 컴파일러 컨텍스트
	 * @return true이면 에러 없이 처리된 것이고, false이면 오류가 발생함.
	 */
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext);

	/**
	 * @brief 출력 페이로드 타입 핀과 입력 페이로드 타입 핀의 연결을 확인하고 갱신합니다.
	 *        두 핀의 타입이 일치하지 않으면, 출력 핀의 타입을 입력 핀에 맞춰 변경합니다.
	 */
	void RefreshOutputPayloadType();

	/**
	 * @brief 출력 페이로드 핀을 반환합니다.
	 * @return 출력용 Payload 핀 (EGPD_Output 방향)
	 */
	UEdGraphPin* GetPayloadPin() const;

	/**
	 * @brief 입력 페이로드 타입 핀을 반환합니다.
	 * @return 입력용 PayloadType 핀 (EGPD_Input 방향)
	 */
	UEdGraphPin* GetPayloadTypePin() const;

	/**
	 * @brief 실제 채널(ActualChannel) 출력 핀을 반환합니다.
	 * @return ActualChannel 핀 (EGPD_Output 방향)
	 */
	UEdGraphPin* GetOutputChannelPin() const;
};

