#pragma once

#include "CoreMinimal.h"

/**
 * ShooterAILog.h
 * 로그 카테고리 선언 및 매크로 정의
 */

// 로그 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(Log_ShooterAI, Log, All);

// 기본 로그 매크로 (함수명, 라인번호와 함께 메시지를 출력)
#define SHOOTERAI_LOG(Verbosity, Format, ...) \
UE_LOG(Log_ShooterAI, Verbosity, TEXT("[%s:%d] %s"), TEXT(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Format), ##__VA_ARGS__))

// 각 Verbosity 별 편의성 매크로
#define SHOOTERAI_LOG_ERROR(Format, ...)    SHOOTERAI_LOG(Error, Format, ##__VA_ARGS__)
#define SHOOTERAI_LOG_WARNING(Format, ...)  SHOOTERAI_LOG(Warning, Format, ##__VA_ARGS__)
#define SHOOTERAI_LOG_DISPLAY(Format, ...)  SHOOTERAI_LOG(Display, Format, ##__VA_ARGS__)
#define SHOOTERAI_LOG_LOG(Format, ...)      SHOOTERAI_LOG(Log, Format, ##__VA_ARGS__)
#define SHOOTERAI_LOG_VERBOSE(Format, ...)  SHOOTERAI_LOG(Verbose, Format, ##__VA_ARGS__)
#define SHOOTERAI_LOG_VVERBOSE(Format, ...) SHOOTERAI_LOG(VeryVerbose, Format, ##__VA_ARGS__)
