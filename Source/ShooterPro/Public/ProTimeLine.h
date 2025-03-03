// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProTimeLine.generated.h"

UCLASS(Blueprintable, BlueprintType)
class SHOOTERPRO_API UProTimeLine : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	TMap<FString, float> Update(float DeltaTime);
	UFUNCTION(BlueprintCallable)
	TMap<FString, float> Reverse(float DeltaTime);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Curve")
	TMap<FString, UCurveFloat*> FloatCurves;

	TMap<FString, float> GetCurveDatas();
	
	float TimeElapsed = 0.f;
	float Duration = 0.f;
};

