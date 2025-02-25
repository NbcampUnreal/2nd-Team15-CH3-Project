// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameplayActors/GSCModularCharacter.h"
#include "ProCharacterBase.generated.h"

class UGSCAbilityInputBindingComponent;
class UGSCCoreComponent;
class UGSCAbilitySystemComponent;
// 메인 캐릭터
// 추가 기능은 컴포넌트를 부착하는 방식으로 구현할 예정
// IAbilitySystemInterface 상속 예정
// 현재로는 PlayerPawnComponent만 있음
UCLASS()
class SHOOTERPRO_API AProCharacterBase : public AGSCModularCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AProCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ProCharacter)
	UGSCCoreComponent* CoreComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ProCharacter)
	UGSCAbilityInputBindingComponent* InputBindingComponent;
};
