// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ProAttributeStatBar.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UProAttributeStatBar::HandleAttributeChange(FGameplayAttribute Attribute, float NewValue, float OldValue)
{
	// 현재 우리가 표시하는 Attribute가 변경되었는지 확인
	if (Attribute == CurrentAttribute)
	{
		if (AttributeProgressBar)
		{
			// 최대 어트리뷰트(예: MaxHealth) 값 가져오기
			float MaxValue = GetAttributeValue(MaxAttribute);
			float Percent = 0.f;

			if (MaxValue > 0.f)
			{
				Percent = NewValue / MaxValue;
			}

			// ProgressBar(게이지)에 반영
			AttributeProgressBar->SetPercent(Percent);
		}

		// 텍스트로 "현재값 / 최대값" 표시
		if (AttributeValueText)
		{
			int32 CurrentRounded = FMath::RoundToInt(NewValue);
			int32 MaxRounded = FMath::RoundToInt(GetAttributeValue(MaxAttribute));
			FString DisplayString = FString::Printf(TEXT("%d / %d"), CurrentRounded, MaxRounded);

			// TextBlock에 표시
			AttributeValueText->SetText(FText::FromString(DisplayString));
		}
	}
}
