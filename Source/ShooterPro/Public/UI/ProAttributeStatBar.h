#pragma once

#include "CoreMinimal.h"
#include "UI/GSCUserWidget.h"
#include "ProAttributeStatBar.generated.h"

class UTextBlock;
class UProgressBar;
/**
 * 체력, 마나, 스태미나 등 여러 어트리뷰트를  프로그래스 바 형태로 표시하기 위한 위젯 클래스 입니다.
 * UGSCUserWidget를 상속받아, HandleAttributeChange 등을 오버라이드하여 어트리뷰트 변화가 있을 때마다 프로그래스 바를 갱신합니다.
 */
UCLASS()
class SHOOTERPRO_API UProAttributeStatBar : public UGSCUserWidget
{
	GENERATED_BODY()

public:
	/** UGSCUserWidget에서 제공하는 어트리뷰트 변경 핸들러를 재정의합니다. */
	virtual void HandleAttributeChange(FGameplayAttribute Attribute, float NewValue, float OldValue) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="UI")
	UTextBlock* AttributeValueText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category="UI")
	UProgressBar* AttributeProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	FGameplayAttribute CurrentAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attributes")
	FGameplayAttribute MaxAttribute;
};
