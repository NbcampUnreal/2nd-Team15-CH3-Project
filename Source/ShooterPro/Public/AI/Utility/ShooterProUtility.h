#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
// #include "ShooterProUtility.generated.h"


/**
 * FDoOnceStruct
 *
 * 한 번 실행되면 이후에는 다시 실행되지 않는 기능을 제공합니다.
 * Reset()을 호출하면 다시 실행 가능한 상태로 복원할 수 있습니다.
 *
 * bStartClosed가 true이면, 구조체 생성 시 이미 실행된 상태로 시작합니다.
 */
struct FDoOnceStruct
{
public:
	bool bHasExecuted;

	FDoOnceStruct(bool bStartClosed = false) : bHasExecuted(bStartClosed)
	{
	}

	template <typename FuncType>
	void Execute(FuncType InFunc)
	{
		if (!bHasExecuted)
		{
			InFunc();
			bHasExecuted = true;
		}
	}

	void Reset()
	{
		bHasExecuted = false;
	}

	bool HasExecuted() const
	{
		return bHasExecuted;
	}
};
