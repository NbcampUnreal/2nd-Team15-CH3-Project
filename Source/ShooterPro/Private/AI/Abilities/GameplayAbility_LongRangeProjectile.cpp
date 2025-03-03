#include "AI/Abilities/GameplayAbility_LongRangeProjectile.h"

#include "Abilities/Tasks/GSCTask_PlayMontageWaitForEvent.h"
#include "AI/Actors/EnemyProjectile.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void UGameplayAbility_LongRangeProjectile::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 코스트/쿨다운 등 Commit
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 내부적으로 EndAbility 할 때 필요하므로 캐싱
	CachedHandle = Handle;
	CachedActorInfo = ActorInfo;
	CachedActivationInfo = ActivationInfo;

	// 만약 애니메이션 없이 바로 던지길 원하면
	if (!ThrowMontage)
	{
		// DoSpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	// =============== 몽타주 재생 태스크 생성 ===============
	UGSCTask_PlayMontageWaitForEvent* MontageTask = UGSCTask_PlayMontageWaitForEvent::PlayMontageAndWaitForEvent(
		this,
		NAME_None,
		ThrowMontage,
		FGameplayTagContainer(),
		1.0f,
		NAME_None,
		true,
		1.0f
	);

	if (!MontageTask)
	{
		// DoSpawnProjectile();
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	// 이벤트 받으면 발사
	MontageTask->EventReceived.AddDynamic(this, &UGameplayAbility_LongRangeProjectile::SpawnProjectileFromEvent);

	// 애니메이션 관련 이벤트
	MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_LongRangeProjectile::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_LongRangeProjectile::OnMontageBlendOut);
	MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_LongRangeProjectile::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_LongRangeProjectile::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

void UGameplayAbility_LongRangeProjectile::SpawnProjectileFromEvent(FGameplayTag EventTag, FGameplayEventData EventData)
{
	if (EventTag == ThrowEventTag)
	{
		DoSpawnProjectile();
	}
}

void UGameplayAbility_LongRangeProjectile::DoSpawnProjectile()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !ProjectileClass)
	{
		return;
	}

	// 1) 소켓(또는 캐릭터 위치)에서 시작점 구하기
	FVector MuzzleLoc = Avatar->GetActorLocation();
	FRotator MuzzleRot = Avatar->GetActorRotation();

	if (const ACharacter* Char = Cast<ACharacter>(Avatar))
	{
		if (const USkeletalMeshComponent* MeshComp = Char->GetMesh())
		{
			FTransform SocketTransform = MeshComp->GetSocketTransform(MuzzleSocketName, RTS_World);
			MuzzleLoc = SocketTransform.GetLocation();
			MuzzleRot = SocketTransform.GetRotation().Rotator();
		}
	}

	// 2) 투사체 액터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = const_cast<AActor*>(Avatar);
	SpawnParams.Instigator = Cast<APawn>(const_cast<AActor*>(Avatar));

	AEnemyProjectile* SpawnedProjectile = Avatar->GetWorld()->SpawnActor<AEnemyProjectile>(
		ProjectileClass,
		MuzzleLoc,
		MuzzleRot,
		SpawnParams
	);

	if (!SpawnedProjectile)
	{
		return;
	}

	// 투사체에 적용할 효과 지정
	SpawnedProjectile->DeBuffEffectClass = DebuffEffectClass;

	// 플레이어 Pawn을 가져옴 (AI 상황이면 Blackboard 등에서 TargetActor를 가져오는 식으로 변경)
	AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);

	// (A) 플레이어가 없거나 ProjectileMovement가 없으면, 그냥 Forward 발사 후 리턴
	if (!PlayerActor || !SpawnedProjectile->GetProjectileMovement())
	{
		FVector DefaultVelocity = MuzzleRot.Vector() * SpawnedProjectile->GetProjectileMovement()->InitialSpeed;
		SpawnedProjectile->GetProjectileMovement()->Velocity = DefaultVelocity;
		return;
	}

	// (B) FSuggestProjectileVelocityParameters 사용 (신규 방식)
	FVector OutVelocity;
	FCollisionResponseParams CollisionParams;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Avatar);

	UGameplayStatics::FSuggestProjectileVelocityParameters Params(
		this, 
		MuzzleLoc,
		PlayerActor->GetActorLocation(),
		SpawnedProjectile->GetProjectileMovement()->InitialSpeed
	);
	Params.bFavorHighArc = false; // 낮은 포물선
	Params.CollisionRadius = 0.f;
	Params.OverrideGravityZ = 0.f; // 0이면 WorldGravity 사용
	Params.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace; // 충돌 무시
	Params.ResponseParam = CollisionParams;
	Params.ActorsToIgnore = IgnoreActors;
	Params.bDrawDebug = false;     // 디버그 궤적 표시

	const bool bFoundValidVelocity = UGameplayStatics::SuggestProjectileVelocity(Params, OutVelocity);

	if (bFoundValidVelocity)
	{
		// 필요하다면 OutVelocity *= 0.7f 등으로 속도 조정
		SpawnedProjectile->GetProjectileMovement()->Velocity = OutVelocity ;
	}
	else
	{
		// 사거리 불충분 등으로 계산 실패 시 fallback
		FVector FallbackVelocity = MuzzleRot.Vector() * SpawnedProjectile->GetProjectileMovement()->InitialSpeed;
		SpawnedProjectile->GetProjectileMovement()->Velocity = FallbackVelocity;
	}
}

void UGameplayAbility_LongRangeProjectile::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, false, false);
}

void UGameplayAbility_LongRangeProjectile::OnMontageBlendOut(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, false, false);
}

void UGameplayAbility_LongRangeProjectile::OnMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}

void UGameplayAbility_LongRangeProjectile::OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CachedHandle, CachedActorInfo, CachedActivationInfo, true, false);
}
