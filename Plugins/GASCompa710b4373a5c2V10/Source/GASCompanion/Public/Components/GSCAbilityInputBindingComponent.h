#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GSCTypes.h"
#include "Components/GSCPlayerControlsComponent.h"
#include "GSCAbilityInputBindingComponent.generated.h"

/**
 * @brief 능력 입력 바인딩 정보를 저장하는 구조체
 *
 * 각 입력 액션에 대해 해당 입력에 연결된 능력들의 스택 및 관련 핸들, 트리거 이벤트 타입 정보를 저장합니다.
 */
USTRUCT()
struct FGSCAbilityInputBinding
{
	GENERATED_BODY()

	// 입력에 대한 고유 식별자. 기본값은 0.
	int32 InputID = 0;

	// 입력이 눌렸을 때 등록된 핸들 ID (Pressed 이벤트 핸들).
	uint32 OnPressedHandle = 0;

	// 입력이 릴리즈되었을 때 등록된 핸들 ID (Released 이벤트 핸들).
	uint32 OnReleasedHandle = 0;

	// 해당 입력에 바인딩된 능력들의 스택. 스택의 Top에는 최신 바인딩된 능력이 위치합니다.
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;

	// 입력 트리거 이벤트 타입. 기본값은 Started (즉, 입력 시작 시 트리거).
	EGSCAbilityTriggerEvent TriggerEvent = EGSCAbilityTriggerEvent::Started;
};

/**
 * @brief 향상된 입력(Enhanced Input)을 능력 시스템(Ability System)과 연결하는 모듈형 Pawn 컴포넌트
 *
 * 이 컴포넌트는 GSCPlayerControlsComponent를 상속받으며, Pawn이 능력을 사용할 경우 해당 컴포넌트를 사용하여
 * 능력 시스템과 입력 바인딩 로직을 연결합니다.
 */
UCLASS(ClassGroup="GASCompanion", meta=(BlueprintSpawnableComponent))
class GASCOMPANION_API UGSCAbilityInputBindingComponent : public UGSCPlayerControlsComponent
{
	GENERATED_BODY()

	// =======================================================================
	// Public 멤버 함수 (인터페이스 및 능력 바인딩 관련)
	// =======================================================================
public:
	// ──────────────────────────────────────────────
	// UPlayerControlsComponent 인터페이스 구현
	// ──────────────────────────────────────────────

	/**
	 * @brief 플레이어 입력 컴포넌트를 설정하는 함수 (구현 함수)
	 * @param PlayerInputComponent 향상된 입력 컴포넌트 포인터
	 */
	virtual void SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent) override;

	/**
	 * @brief 이전 컨트롤러가 교체될 때 입력 컴포넌트를 해제하는 함수
	 * @param OldController 이전에 소유했던 컨트롤러 포인터
	 */
	virtual void ReleaseInputComponent(AController* OldController) override;


	// ──────────────────────────────────────────────
	// 능력 부여 및 입력 바인딩 함수
	// ──────────────────────────────────────────────

	/**
	 * @brief ASC(Ability System Component)에 능력을 부여하고, 선택적으로 입력 액션과 트리거 이벤트를 바인딩하는 함수
	 *
	 * 서버에서 실행되어 ASC->GiveAbility()를 호출한 후, 클라이언트에서 입력 바인딩을 설정합니다.
	 * Pawn 초기화 시 수동으로 능력을 부여할 때 사용하며,
	 * 비 PlayerState Pawn은 Possessed 이벤트, PlayerState Pawn은 Authority의 OnInitAbilityActorInfo에서 호출합니다.
	 *
	 * @param Ability 부여할 Gameplay Ability 클래스
	 * @param Level 부여할 능력의 레벨 (기본값: 1)
	 * @param InputAction 능력을 바인딩할 입력 액션 (옵션, nullptr 허용)
	 * @param TriggerEvent 입력 액션이 있을 경우 사용할 트리거 이벤트 타입 (기본값: Started)
	 * @return 부여된 능력을 식별하기 위한 Gameplay Ability Spec 핸들
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS Companion|Abilities")
	FGameplayAbilitySpecHandle GiveAbilityWithInput(
		const TSubclassOf<UGameplayAbility> Ability,
		const int32 Level = 1,
		UInputAction* InputAction = nullptr,
		const EGSCAbilityTriggerEvent TriggerEvent = EGSCAbilityTriggerEvent::Started
	);

	/**
	 * @brief 서버에서 ASC->GiveAbility()로 부여된 능력 핸들을 클라이언트에서 입력 액션과 연결하기 위한 RPC 헬퍼 함수
	 *
	 * @param InInputAction 클라이언트에서 바인딩할 입력 액션
	 * @param InTriggerEvent 입력 액션의 트리거 이벤트 타입
	 * @param InAbilityHandle 부여된 능력의 핸들
	 */
	UFUNCTION(Client, Reliable)
	void ClientBindInput(UInputAction* InInputAction, const EGSCAbilityTriggerEvent InTriggerEvent, const FGameplayAbilitySpecHandle& InAbilityHandle);

	/**
	 * @brief 입력 바인딩 컴포넌트에 등록된 바인딩을 업데이트하거나, 새로운 능력 핸들에 대해 바인딩을 생성하는 함수
	 *
	 * @param InputAction 바인딩할 Enhanced Input 액션
	 * @param TriggerEvent 입력 액션이 눌렸을 때 사용할 트리거 이벤트 타입 (일반적으로 Started)
	 * @param AbilityHandle 능력 부여 시 반환된 Gameplay Ability Spec 핸들
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	void SetInputBinding(UInputAction* InputAction, EGSCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpecHandle AbilityHandle);

	/**
	 * @brief 특정 Gameplay Ability Spec 핸들에 대해 등록된 입력 바인딩을 해제하고 초기화하는 함수
	 * @param AbilityHandle 해제할 능력의 Gameplay Ability Spec 핸들
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	/**
	 * @brief 특정 Enhanced Input 액션에 대해 등록된 입력 바인딩 델리게이트(Pressed, Released)를 해제하고,
	 * 연결된 능력들의 InputID를 초기화하는 함수
	 * @param InputAction 해제할 입력 액션
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	void ClearAbilityBindings(UInputAction* InputAction);

	/**
	 * @brief Gameplay Ability 인스턴스를 전달받아, 해당 능력의 InputID와 매핑된 입력 액션을 반환하는 함수
	 * 주로 Gameplay Ability 이벤트 그래프 내에서 self 참조를 통해 호출합니다.
	 * @param Ability 조회할 Gameplay Ability (참조형)
	 * @return 해당 능력과 연결된 입력 액션 포인터 (연결되지 않았으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Abilities")
	UInputAction* GetBoundInputActionForAbility(UPARAM(ref) const UGameplayAbility* Ability);

	/**
	 * @brief Gameplay Ability 클래스에 대해, 해당 클래스의 기본 객체에 연결된 입력 액션을 반환하는 함수
	 * 임의의 위치에서 Gameplay Ability 클래스 참조를 전달하여 호출할 수 있습니다.
	 * @param InAbilityClass 조회할 Gameplay Ability 클래스
	 * @return 해당 클래스에 바인딩된 입력 액션 포인터 (연결되지 않았으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Abilities")
	UInputAction* GetBoundInputActionForAbilityClass(TSubclassOf<UGameplayAbility> InAbilityClass);

	/**
	 * @brief 내부 헬퍼 함수로, 주어진 Ability Spec의 InputID와 매핑된 입력 액션을 반환합니다.
	 *
	 * @param AbilitySpec 조회할 Gameplay Ability Spec 포인터
	 * @return 매핑된 입력 액션 포인터 (연결되지 않았으면 nullptr)
	 */
	UInputAction* GetBoundInputActionForAbilitySpec(const FGameplayAbilitySpec* AbilitySpec) const;


	// =======================================================================
	// Public UPROPERTY (구성 요소 및 설정 관련)
	// =======================================================================
public:
	/// @brief ASC(Ability System Component)에서 타겟 확인을 위한 입력 액션 (읽기 전용)
	UPROPERTY(EditDefaultsOnly, Category = "Player Controls", meta = (DisplayAfter = "InputPriority"))
	TObjectPtr<UInputAction> TargetInputConfirm;

	/**
	 * @brief 타겟 확인 입력에 사용할 Enhanced Input Trigger 이벤트 타입
	 * 일반적으로 단 한 번의 동작(버튼 누름 즉시 실행)인 경우 Started,
	 * 지속 동작(버튼을 누르고 있는 동안 매 프레임 실행)인 경우 Triggered를 사용합니다.
	 *
	 * @warning Triggered는 매 틱 실행 시 문제를 야기할 수 있으므로, 보통 Started를 권장합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Controls", meta = (DisplayAfter = "InputPriority", EditCondition = "TargetInputConfirm != nullptr", EditConditionHides))
	EGSCAbilityTriggerEvent TargetConfirmTriggerEvent = EGSCAbilityTriggerEvent::Started;

	/// @brief ASC에서 타겟 취소를 위한 입력 액션 (읽기 전용)
	UPROPERTY(EditDefaultsOnly, Category = "Player Controls", meta = (DisplayAfter = "InputPriority"))
	TObjectPtr<UInputAction> TargetInputCancel;

	/**
	 * @brief 타겟 취소 입력에 사용할 Enhanced Input Trigger 이벤트 타입
	 * 일반적으로 단 한 번의 동작(버튼 누름 즉시 실행)인 경우 Started,
	 * 지속 동작(버튼을 누르고 있는 동안 매 프레임 실행)인 경우 Triggered를 사용합니다.
	 * @warning Triggered는 단 한 번만 실행되어야 하며, 그렇지 않으면 능력 실행에 문제가 발생할 수 있습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Controls", meta = (DisplayAfter = "InputPriority", EditCondition = "TargetInputCancel != nullptr", EditConditionHides))
	EGSCAbilityTriggerEvent TargetCancelTriggerEvent = EGSCAbilityTriggerEvent::Started;


	// =======================================================================
	// Private 멤버 (내부 로직 및 상태 관리)
	// =======================================================================
private:
	// ──────────────────────────────────────────────
	// 내부 함수 (유틸리티 / 이벤트 처리 / 초기화)
	// ──────────────────────────────────────────────

	/// @brief 입력 바인딩 관련 내부 상태를 초기화하는 함수
	void ResetBindings();

	/// @brief ASC 관련 설정 실행 (ASC 참조 획득 및 각 능력의 InputID 업데이트)
	void RunAbilitySystemSetup();

	/**
	 * @brief 입력(Pressed/Released) 이벤트 발생 시, 매핑된 능력의 InputID를 업데이트합니다.
	 *        (PIE 세션 후 클라이언트에서 InputID 소실 문제 해결용)
	 *
	 * @param AbilitySystemComponent 업데이트에 사용할 ASC 포인터
	 */
	void UpdateAbilitySystemBindings(UAbilitySystemComponent* AbilitySystemComponent);

	/// @brief 입력 액션이 눌렸을 때 호출되는 이벤트 처리 함수
	void OnAbilityInputPressed(UInputAction* InputAction);

	/// @brief 입력 액션이 릴리즈되었을 때 호출되는 이벤트 처리 함수
	void OnAbilityInputReleased(UInputAction* InputAction);

	/// @brief 타겟 확인 입력 처리 함수
	void OnLocalInputConfirm();

	/// @brief 타겟 취소 입력 처리 함수
	void OnLocalInputCancel();

	/// @brief 주어진 입력 액션에 대한 매핑 정보를 삭제하는 내부 함수
	void RemoveEntry(const UInputAction* InputAction);

	/**
	 * @brief 주어진 핸들을 통해 ASC 내에서 해당하는 Gameplay Ability Spec을 찾는 함수
	 *
	 * @param Handle 조회할 Ability Spec 핸들
	 * @return 찾은 Gameplay Ability Spec 포인터 (없으면 nullptr)
	 */
	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle) const;

	/// @brief 주어진 입력 액션에 대해 능력 입력 바인딩을 설정하는 내부 함수
	void TryBindAbilityInput(UInputAction* InputAction, FGSCAbilityInputBinding& AbilityInputBinding);

	/**
	 * @brief 내부적으로 Enhanced Input의 Trigger 이벤트 타입을 변환하여 반환하는 함수
	 *
	 * @param TriggerEvent 내부 정의된 EGSCAbilityTriggerEvent 값
	 * @return 변환된 ETriggerEvent 값
	 */
	static ETriggerEvent GetInputActionTriggerEvent(EGSCAbilityTriggerEvent TriggerEvent);


	// ──────────────────────────────────────────────
	// 내부 변수 (상태 저장 및 구성 요소)
	// ──────────────────────────────────────────────

	/// @brief ASC(Ability System Component) 참조 (입력 바인딩 시 사용)
	UPROPERTY(transient)
	TObjectPtr<UAbilitySystemComponent> AbilityComponent;

	/// @brief 입력 액션과 능력 바인딩 정보를 저장하는 맵
	UPROPERTY(transient)
	TMap<TObjectPtr<UInputAction>, FGSCAbilityInputBinding> MappedAbilities;

	/// @brief 타겟 확인 입력에 대한 바인딩 핸들
	uint32 OnConfirmHandle = 0;

	/// @brief 타겟 취소 입력에 대한 바인딩 핸들
	uint32 OnCancelHandle = 0;

	/// @brief 컴포넌트에서 등록한 모든 향상된 입력 핸들의 리스트
	TArray<uint32> RegisteredInputHandles;
};
