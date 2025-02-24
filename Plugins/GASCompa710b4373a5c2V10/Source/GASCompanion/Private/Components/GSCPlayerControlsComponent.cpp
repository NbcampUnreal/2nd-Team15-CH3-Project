
#include "Components/GSCPlayerControlsComponent.h"

#include "EnhancedInputSubsystems.h"
#include "GSCLog.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void UGSCPlayerControlsComponent::OnRegister()
{
	Super::OnRegister();

    // 현재 월드와 소유 Pawn을 가져옴
	const UWorld* World = GetWorld();
	APawn* MyOwner = GetPawn<APawn>();

    // Pawn과 World가 유효하고, 실제 게임 월드인 경우에만 작업을 수행
	if (ensure(MyOwner) && World->IsGameWorld())
	{
        // Pawn의 재시작 이벤트 델리게이트에 OnPawnRestarted() 함수를 등록합니다.
		if (!MyOwner->ReceiveRestartedDelegate.Contains(this, TEXT("OnPawnRestarted")))
		{
			MyOwner->ReceiveRestartedDelegate.AddDynamic(this, &UGSCPlayerControlsComponent::OnPawnRestarted);
		}

        // Pawn의 컨트롤러 변경 이벤트 델리게이트에 OnControllerChanged() 함수를 등록합니다.
		if (!MyOwner->ReceiveControllerChangedDelegate.Contains(this, TEXT("OnControllerChanged")))
		{
			MyOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &UGSCPlayerControlsComponent::OnControllerChanged);
		}

        // 만약 Pawn에 이미 입력 컴포넌트가 존재하면, 재시작 이벤트를 강제로 호출하여 입력 구성을 갱신합니다.
		if (MyOwner->InputComponent)
		{
			OnPawnRestarted(MyOwner);
		}
	}
}

void UGSCPlayerControlsComponent::OnUnregister()
{
    // 현재 월드를 가져옴
	const UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
        // 입력 컴포넌트를 해제하여 바인딩된 입력을 모두 정리합니다.
		ReleaseInputComponent();

        // 소유 Pawn을 가져와, 등록된 모든 델리게이트를 제거합니다.
        if (APawn* MyOwner = GetPawn<APawn>())
		{
			MyOwner->ReceiveRestartedDelegate.RemoveAll(this);
			MyOwner->ReceiveControllerChangedDelegate.RemoveAll(this);
		}
	}

	Super::OnUnregister();
}

void UGSCPlayerControlsComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
    // 기본 구현: 별도의 입력 액션 바인딩은 하지 않음.
}

void UGSCPlayerControlsComponent::TeardownPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
    // 기본 구현: 별도의 입력 액션 해제 로직 없음.
}

void UGSCPlayerControlsComponent::OnPawnRestarted(APawn* Pawn)
{
    GSC_LOG(Verbose, TEXT("UGSCPlayerControlsComponent::OnPawnRestarted Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"));

    // Pawn이 유효하고, 이 컴포넌트의 소유자와 동일하며, 입력 컴포넌트가 존재하는 경우
	if (ensure(Pawn && Pawn == GetOwner()) && Pawn->InputComponent)
	{
        // 기존 입력 컴포넌트를 해제하여 이전 바인딩을 제거합니다.
		ReleaseInputComponent();

        // 새로 생성된 입력 컴포넌트가 존재하면, 입력 설정을 다시 수행합니다.
		if (Pawn->InputComponent)
		{
			SetupInputComponent(Pawn);
		}
	}
}

void UGSCPlayerControlsComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
    // 로그: 컨트롤러 변경 이벤트 발생 (한글 메시지)
    GSC_LOG(Verbose, TEXT("UGSCPlayerControlsComponent::OnControllerChanged Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"));
    
    // Pawn이 유효하며, 이 컴포넌트의 소유자이고, 이전 컨트롤러가 존재하는 경우에만 입력 컴포넌트를 해제합니다.
	if (ensure(Pawn && Pawn == GetOwner()) && OldController)
	{
		ReleaseInputComponent(OldController);
	}
}

void UGSCPlayerControlsComponent::SetupInputComponent(APawn* Pawn)
{
    // Pawn의 InputComponent를 UEnhancedInputComponent로 캐스팅합니다.
	InputComponent = Cast<UEnhancedInputComponent>(Pawn->InputComponent);

    // 입력 컴포넌트가 유효한지 확인합니다.
	if (ensureMsgf(InputComponent, TEXT("프로젝트는 PlayerControlsComponent를 지원하기 위해 EnhancedInputComponent를 사용해야 합니다.")))
	{
        // 로컬 플레이어 서브시스템을 가져옵니다.
		UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();

        // 서브시스템과 입력 맵핑 컨텍스트가 유효하면, 입력 맵핑을 추가합니다.
		if (Subsystem && InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, InputPriority);
		}
		
        GSC_LOG(Verbose, TEXT("UGSCPlayerControlsComponent::SetupInputComponent Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"));

        // 실제 입력 액션 바인딩 등의 추가 설정을 수행하는 SetupPlayerControls() 함수를 호출합니다.
		SetupPlayerControls(InputComponent);
	}
}

void UGSCPlayerControlsComponent::ReleaseInputComponent(AController* OldController)
{
    // 현재 또는 이전 컨트롤러를 기준으로 로컬 플레이어 서브시스템을 가져옵니다.
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OldController);
	if (Subsystem && InputComponent)
	{
        // 입력 컴포넌트 해제에 필요한 추가 정리 작업을 수행하는 TeardownPlayerControls() 함수를 호출합니다.
		TeardownPlayerControls(InputComponent);

        // 입력 맵핑 컨텍스트가 존재한다면 입력 시스템에서 제거합니다.
		if (InputMappingContext)
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
	}
    // 입력 컴포넌트 포인터를 nullptr로 초기화합니다.
	InputComponent = nullptr;
}

UEnhancedInputLocalPlayerSubsystem* UGSCPlayerControlsComponent::GetEnhancedInputSubsystem(AController* OldController) const
{
    // Pawn이 존재하는지 확인합니다.
	if (!GetPawn<APawn>())
	{
		return nullptr;
	}

    // 현재 컨트롤러를 가져옵니다.
	const APlayerController* PC = GetController<APlayerController>();
    // 현재 컨트롤러가 없다면, OldController를 사용합니다.
	if (!PC)
	{
		PC = Cast<APlayerController>(OldController);
		if (!PC)
		{
			return nullptr;
		}
	}

    // 로컬 플레이어를 가져옵니다.
	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return nullptr;
	}

    // 로컬 플레이어에서 Enhanced Input 로컬 플레이어 서브시스템을 반환합니다.
	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
