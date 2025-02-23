// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GSCDeveloperSettings.generated.h"

/**
 * @brief GAS Companion 플러그인의 일반 설정을 관리하는 클래스.
 *
 * 이 클래스는 GAS Companion 플러그인의 전반적인 설정값들을 저장합니다.
 * 게임의 구성 파일(Game Config)에서 읽어들이며, 주로 UAbilitySystemGlobals의 초기화 타이밍과 관련된 설정을 제어합니다.
 * 사용자가 이 옵션을 활성화하면 플러그인의 StartupModule 메서드에서 UAbilitySystemGlobals의 InitGlobalData 호출을 방지할 수 있습니다.
 *
 * @note 이 클래스는 게임의 기본 설정(Config) 파일에 저장되므로, 프로젝트 설정에서 관련 옵션을 변경할 수 있습니다.
 */
UCLASS(Config="Game", defaultconfig, meta=(DisplayName="GAS Companion"))
class GASCOMPANION_API UGSCDeveloperSettings : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief UGSCDeveloperSettings의 생성자.
	 *
	 * FObjectInitializer를 사용하여 기본 멤버 변수들을 초기화합니다.
	 *
	 * @param ObjectInitializer 기본 초기화 객체.
	 */
	UGSCDeveloperSettings(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief GAS Companion 모듈이 StartupModule에서 UAbilitySystemGlobals의 InitGlobalData를 초기화하지 않도록 할지 결정합니다.
	 *
	 * 이 옵션이 true로 설정되면, GAS Companion의 StartupModule에서 UAbilitySystemGlobals::InitGlobalData() 호출을 방지합니다.
	 * 기본 엔진의 초기화 타이밍에 따라 InitGlobalData()가 너무 일찍 호출되는 문제를 회피하기 위해 사용되며,
	 * 이를 위해 AssetManager 서브클래스를 사용하고, 해당 서브클래스 내의 StartInitialLoading 시점에 InitGlobalData()가 호출되도록 해야 합니다.
	 *
	 * 사용 시 주의사항:
	 * - "Project Settings -> Engine > General Settings > Asset Manager Class"에서 사용자가 만든 AssetManager 서브클래스로 변경해야 합니다.
	 * - GAS Companion은 GSCAssetManager를 제공하며, 현재 Asset Manager 클래스가 엔진 기본값일 경우 에디터에서 변경할지 묻는 메시지를 표시합니다.
	 *
	 * @return bool true이면 전역 데이터 초기화를 플러그인 StartupModule에서 방지, false이면 기본 초기화를 수행합니다.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Ability System", meta=(DisplayName = "Prevent Ability System Global Data Initialization in Startup Module (Recommended)"))
	bool bPreventGlobalDataInitialization = false;
};
