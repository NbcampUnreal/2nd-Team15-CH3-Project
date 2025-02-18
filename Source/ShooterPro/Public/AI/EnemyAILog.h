#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"  // GEngine, AddOnScreenDebugMessage 등을 사용하기 위한 헤더

/**
 * ShooterAILog.h
 * 로그 카테고리 선언 및 매크로 정의
 */

// 로그 카테고리 선언
DECLARE_LOG_CATEGORY_EXTERN(Log_EnemyAI, Log, All);

// 기본 로그 매크로 (함수명, 라인번호와 함께 메시지를 출력)
#define ENEMY_AI_LOG(Verbosity, Format, ...) \
    UE_LOG(Log_EnemyAI, Verbosity, TEXT("[%s:%d] %s"), TEXT(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Format), ##__VA_ARGS__))

// 각 Verbosity 별 편의성 매크로 (콘솔 로그용)
#define AI_ENEMY_LOG_ERROR(Format, ...)    ENEMY_AI_LOG(Error, Format, ##__VA_ARGS__)
#define AI_ENEMY_LOG_WARNING(Format, ...)  ENEMY_AI_LOG(Warning, Format, ##__VA_ARGS__)
#define AI_ENEMY_LOG_DISPLAY(Format, ...)  ENEMY_AI_LOG(Display, Format, ##__VA_ARGS__)
#define AI_ENEMY_LOG_LOG(Format, ...)      ENEMY_AI_LOG(Log, Format, ##__VA_ARGS__)

// ------------------------------
// 스크린에 메시지를 출력하는 매크로
// ------------------------------
#define AI_ENEMY_SCREEN_LOG_ERROR(Format, ...)  \
{   \
FString Message = FString::Printf(TEXT("[%s:%d] %s"), TEXT(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
UE_LOG(Log_EnemyAI, Error, TEXT("%s"), *Message); \
if (GEngine) { \
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message); \
} \
}

#define AI_ENEMY_SCREEN_LOG_WARNING(Format, ...)  \
{   \
FString Message = FString::Printf(TEXT("[%s:%d] %s"), TEXT(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
UE_LOG(Log_EnemyAI, Warning, TEXT("%s"), *Message); \
if (GEngine) { \
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Message); \
} \
}

#define AI_ENEMY_SCREEN_LOG_LOG(Format, ...)  \
{   \
FString Message = FString::Printf(TEXT("[%s:%d] %s"), TEXT(__FUNCTION__), __LINE__, *FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
UE_LOG(Log_EnemyAI, Log, TEXT("%s"), *Message); \
if (GEngine) { \
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, Message); \
} \
}
