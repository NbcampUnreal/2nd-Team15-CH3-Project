// Fill out your copyright notice in the Description page of Project Settings.


#include "ProTimeLine.h"

TMap<FString, float> UProTimeLine::Update(float DeltaTime)
{
	TimeElapsed += DeltaTime;
	
	return GetCurveDatas();
}

TMap<FString, float> UProTimeLine::Reverse(float DeltaTime)
{
	TimeElapsed -= DeltaTime;

	return GetCurveDatas();
}


TMap<FString, float> UProTimeLine::GetCurveDatas()
{
	TMap<FString, float> CurveValues;

	for (const TPair<FString, UCurveFloat*>& Pair : FloatCurves)
	{
		if (Pair.Value)
		{
			float CurveValue = Pair.Value->GetFloatValue(TimeElapsed);
			CurveValues.Add(Pair.Key, CurveValue);
		}
	}

	return CurveValues;
}