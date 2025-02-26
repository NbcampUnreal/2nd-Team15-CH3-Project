// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "AIOptimizerComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTERPRO_API UAIOptimizerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAIOptimizerComponent();

	virtual void BeginPlay() override;

	/**
	* Bitmasks can be used to disable features for optimization.
	* 비트마스크를 활용해 최적화를 위한 기능 비활성화가 가능합니다.
	*/
	UFUNCTION(BlueprintCallable)
	void OptimizerSetting(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/ShooterPro.EAIOptimizerFlags"))int32 OptimizerEnable);
	//ACharacter* Character, AAIController* AIC, 
protected:
	// A short range layer that recognizes players. Not disabled.
	// 플레이어를 인식하는 짧은 범위의 레이어. 기능 비활성화 않음.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Radius")
	float LayerShort;
	// Mid-range layer that recognizes the player. Shadows, Actor ticks disabled.
	// 플레이어를 인식하는 중간 범위의 레이어. 그림자, 액터 틱 비활성화.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Radius")
	float LayerMiddle;
	// Long range layer that recognizes the player. Disable all features except animation, collision.
	// 플레이어를 인식하는 긴 범위의 레이어. 애니메이션, 콜리전 제외한 모든 기능 비활성화.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer Radius")
	float LayerLong;

protected:
	// Stop the behavior tree
	// 비헤이비어 트리 중지
	void SetAILogicEnabled(ACharacter* Character, AAIController* AIC, bool bEnable);
	// Stop the character movement
	// 캐릭터 무브먼트 중지
	void SetCharacterMovementEnabled(ACharacter* Character, bool bEnable);

	// Checking the distance to the player and determining which layers to run
	// 플레이어와의 거리를 확인하고 어떤 레이어를 실행시킬지 정함 
	void LayerCheckLoop();
	// Calculating distance to players
	// 플레이어와의 거리 계산
	int32 DistanceLayer();

	FTimerHandle LayerCheckLoopTimer;
};
