// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/ProBulletGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"

AProBulletGrenade::AProBulletGrenade()
{
    PrimaryActorTick.bCanEverTick = true;

    //ProjectileMovement->ProjectileGravityScale = 1.0f; // 중력 영향 (1.0 = 기본 중력, 0.0 = 중력 무시)
    //NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
    //NiagaraComp->SetupAttachment(RootComponent);
    //NiagaraComp->OnSystemFinished.AddDynamic(this, &AProBulletGrenade::OnNiagaraEffectFinished);
    //ExplosionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    //ExplosionSphere->SetupAttachment(RootComponent);


}

void AProBulletGrenade::BeginPlay()
{
    Super::BeginPlay();
}

//void AProBulletGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
//    FVector NormalImpulse, const FHitResult& Hit)
//{
//    if (!OtherActor || OtherActor == AvatarActor) return;
//
//    // 타이머 해제 후 Detonate 바로 호출
//    GetWorld()->GetTimerManager().ClearTimer(ExplodeTimer);
//    Detonate();
//}
//
//void AProBulletGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
//{
//}


void AProBulletGrenade::ActivateBullet(AActor* Avatar, const FVector& SpawnLocation, const FRotator& SpawnRotation,
    const FVector& Direction, const float Speed)
{
    //Super::Activate(Avatar, SpawnLocation, SpawnRotation, Direction, Speed);
    SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);

    AvatarActor = Avatar;
	
    // ProjectileMovement를 재설정
    ProjectileMovement->Velocity = Direction * Speed; // 원하는 방향 * 속도
    ProjectileMovement->Activate(); // 혹시나 비활성화되었을 수도 있으므로 활성화
    ProjectileMovement->SetUpdatedComponent(CollisionComp);
	
    CollisionComp->IgnoreActorWhenMoving(AvatarActor, true);
    
    // ExplodeTimer 설정 -> ExplodeTime 후 Detonate 호출
    //GetWorld()->GetTimerManager().SetTimer(ExplodeTimer, this, &AProBulletGrenade::Detonate, ExplodeTime, false);
    
}

void AProBulletGrenade::OnNiagaraEffectFinished(UNiagaraComponent* PSystem)
{
    DeactivateBullet();
}
