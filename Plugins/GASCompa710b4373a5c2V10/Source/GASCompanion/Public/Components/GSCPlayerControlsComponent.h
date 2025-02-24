#pragma once

#include "CoreMinimal.h"

#include "EnhancedInputComponent.h"
#include "InputTriggers.h"
#include "Components/PawnComponent.h"
#include "GSCPlayerControlsComponent.generated.h"

// 전방 선언: 향상된 입력 관련 서브시스템 및 입력 맵핑, 액션 클래스
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;

/**
 * @brief Pawn에 입력 액션과 선택적인 입력 맵핑을 추가하기 위한 모듈형 컴포넌트
 *
 * 이 컴포넌트는 Pawn에 입력 관련 기능을 제공하며, 입력 액션과 매핑을 동적으로 관리합니다.
 * 만약 Pawn이 능력(Ability) 관련 로직을 처리한다면, GSCAbilityInputBindingComponent를 사용해야 합니다.
 * 이 컴포넌트는 블루프린트에서 직접 추가하기보다는 기본 클래스로 사용하여 상속받아 확장하는 방식으로 사용됩니다.
 */
UCLASS(ClassGroup="GASCompanion", Category = "Input")
class GASCOMPANION_API UGSCPlayerControlsComponent : public UPawnComponent
{
	GENERATED_BODY()

	// =======================================================================
	// Public 멤버 함수 및 변수 (인터페이스 제공 및 기본 설정)
	// =======================================================================
public:
	// ──────────────────────────────────────────────
	// UActorComponent 인터페이스 재정의 (컴포넌트 등록/해제 관련)
	// ──────────────────────────────────────────────

	/**
	 * @brief 컴포넌트가 등록될 때 호출됩니다.
	 *        여기서는 Pawn의 입력 구성 및 동적 델리게이트 등록 등을 수행합니다.
	 */
	virtual void OnRegister() override;

	/**
	 * @brief 컴포넌트가 해제될 때 호출됩니다.
	 *        등록된 입력 컴포넌트 및 델리게이트를 정리합니다.
	 */
	virtual void OnUnregister() override;


	// =======================================================================
	// Protected 멤버 함수 (입력 구성 및 이벤트 핸들러)
	// =======================================================================
protected:
	// ──────────────────────────────────────────────
	// 기본 입력 설정 이벤트 (네이티브/블루프린트 오버라이드 가능)
	// ──────────────────────────────────────────────

	/**
	 * @brief 플레이어 컨트롤 설정을 위한 네이티브/블루프린트 이벤트 함수.
	 *        실제 입력 컴포넌트(UEnhancedInputComponent)가 준비되었을 때, 입력 액션 바인딩 등을 설정합니다.
	 *
	 * @param PlayerInputComponent 입력 컴포넌트 포인터
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Player Controls")
	void SetupPlayerControls(UEnhancedInputComponent* PlayerInputComponent);

	/**
	 * @brief 입력 설정을 해제하기 위한 네이티브/블루프린트 이벤트 함수.
	 *        입력 컴포넌트의 바인딩 해제 등 정리 작업을 수행합니다.
	 *
	 * @param PlayerInputComponent 입력 컴포넌트 포인터
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Player Controls")
	void TeardownPlayerControls(UEnhancedInputComponent* PlayerInputComponent);


	// ──────────────────────────────────────────────
	// 입력 액션 바인딩 헬퍼 함수
	// ──────────────────────────────────────────────

	/**
	 * @brief 주어진 입력 액션을 현재 바인딩된 입력 컴포넌트에 연결하는 래퍼 함수.
	 *
	 * @tparam UserClass 바인딩할 객체의 클래스 타입
	 * @tparam FuncType 바인딩할 멤버 함수의 타입
	 * @param Action 바인딩할 입력 액션 (nullptr이 아니어야 함)
	 * @param EventType 입력 액션 발생 시 트리거 이벤트 타입 (Pressed, Released 등)
	 * @param Object 입력 액션을 처리할 객체 포인터
	 * @param Func 객체에서 실행할 멤버 함수
	 * @return 바인딩 성공 여부 (입력 컴포넌트와 액션이 유효해야 true 반환)
	 */
	template <class UserClass, typename FuncType>
	bool BindInputAction(const UInputAction* Action, const ETriggerEvent EventType, UserClass* Object, FuncType Func)
	{
		if (ensure(InputComponent != nullptr) && ensure(Action != nullptr))
		{
			InputComponent->BindAction(Action, EventType, Object, Func);
			return true;
		}

		return false;
	}


	// ──────────────────────────────────────────────
	// Pawn 및 컨트롤러 상태 변경 이벤트 처리 함수
	// ──────────────────────────────────────────────

	/**
	 * @brief Pawn이 재시작될 때 호출되는 이벤트 처리 함수.
	 *        Pawn이 재시작되면 입력 컴포넌트를 다시 설정하도록 처리합니다.
	 *
	 * @param Pawn 재시작된 Pawn 포인터
	 */
	UFUNCTION()
	virtual void OnPawnRestarted(APawn* Pawn);

	/**
	 * @brief 컨트롤러가 변경될 때 호출되는 이벤트 처리 함수.
	 *        이전 컨트롤러에서 해제하고, 필요한 경우 새 컨트롤러로 입력 컴포넌트를 이전합니다.
	 *
	 * @param Pawn Pawn 포인터
	 * @param OldController 이전 컨트롤러 포인터
	 * @param NewController 새 컨트롤러 포인터
	 */
	UFUNCTION()
	virtual void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);


	// ──────────────────────────────────────────────
	// 입력 컴포넌트 설정 및 해제 관련 함수
	// ──────────────────────────────────────────────

	/**
	 * @brief Pawn의 입력 컴포넌트를 설정하는 함수.
	 *        입력 컴포넌트를 캐스팅하고, 입력 맵핑 컨텍스트를 추가한 후 SetupPlayerControls()를 호출합니다.
	 *
	 * @param Pawn 입력 컴포넌트를 설정할 Pawn 포인터
	 */
	virtual void SetupInputComponent(APawn* Pawn);

	/**
	 * @brief 입력 컴포넌트를 해제하는 함수.
	 *        기존에 바인딩한 입력 액션 및 맵핑 컨텍스트를 제거하고, InputComponent 포인터를 초기화합니다.
	 *
	 * @param OldController 선택 사항: 이전 컨트롤러 포인터 (없을 경우 기본 nullptr)
	 */
	virtual void ReleaseInputComponent(AController* OldController = nullptr);

	/**
	 * @brief 향상된 입력 로컬 플레이어 서브시스템을 반환하는 함수.
	 *        입력 맵핑 및 액션 처리를 위해 현재 컨트롤러 또는 이전 컨트롤러를 기준으로 서브시스템을 가져옵니다.
	 *
	 * @param OldController 선택 사항: 이전 컨트롤러 포인터 (없을 경우 기본 nullptr)
	 * @return UEnhancedInputLocalPlayerSubsystem 포인터 (없으면 nullptr)
	 */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem(AController* OldController = nullptr) const;

public:
	// ──────────────────────────────────────────────
	// 입력 시스템에 추가할 입력 맵핑 및 우선순위 설정
	// ──────────────────────────────────────────────

	/**
	 * @brief 입력 시스템에 추가할 입력 맵핑 컨텍스트.
	 *        이 맵핑은 Pawn의 입력을 처리하기 위해 사용되며, 다양한 입력 액션을 포함할 수 있습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Controls")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	/**
	 * @brief 입력 맵핑 컨텍스트를 바인딩할 때 사용할 우선순위.
	 *        값이 클수록 우선적으로 처리되며, 여러 입력 맵핑이 있을 때 우선순위를 조정하는데 사용됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Controls")
	int32 InputPriority = 0;

	// =======================================================================
	// Protected 멤버 변수 (입력 컴포넌트 참조)
	// =======================================================================
protected:
	/**
	 * @brief Pawn에 바인딩된 입력 컴포넌트.
	 *        UEnhancedInputComponent 타입으로 캐스팅되어 입력 액션 바인딩 및 맵핑 컨텍스트 관리에 사용됩니다.
	 *        이 변수는 컴포넌트가 등록되는 동안에만 유효합니다.
	 */
	UPROPERTY(transient)
	TObjectPtr<UEnhancedInputComponent> InputComponent;
};
