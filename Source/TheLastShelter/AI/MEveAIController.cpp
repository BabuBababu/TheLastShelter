// Copyright TheLastShelter. All Rights Reserved.

#include "MEveAIController.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "MPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

AMEveAIController::AMEveAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMEveAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AMEveAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	OwnerEve = Cast<AMEveCharacter>(InPawn);
}

void AMEveAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!OwnerEve || OwnerEve->IsDead()) return;

	// 1) 적(Ordo) 감지 시 → 접근 & 공격
	if (DetectedTarget && DetectedTarget->IsA<AMOrdoCharacter>())
	{
		AMOrdoCharacter* TargetOrdo = Cast<AMOrdoCharacter>(DetectedTarget);
		if (TargetOrdo && !TargetOrdo->IsDead())
		{
			const float DistToTarget = FVector::Dist(OwnerEve->GetActorLocation(), DetectedTarget->GetActorLocation());

			if (DistToTarget <= AttackRange)
			{
				StopMovement();
				// 쿨다운 체크 후 공격
				const float Now = GetWorld()->GetTimeSeconds();
				if (Now - LastAttackTime >= AttackRate)
				{
					OwnerEve->PerformAttack(DetectedTarget);
					LastAttackTime = Now;
				}
			}
			else
			{
				MoveToActor(DetectedTarget, AttackRange * 0.8f);
			}
			return;
		}
	}

	// 2) 적 없으면 → 플레이어 근처에서 배회
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// 플레이어가 멀면 먼저 따라가기
	if (PlayerPawn)
	{
		const float DistToPlayer = FVector::Dist(OwnerEve->GetActorLocation(), PlayerPawn->GetActorLocation());
		if (DistToPlayer > FollowDistance)
		{
			IsWandering = false;
			IsWanderIdle = false;
			MoveToActor(PlayerPawn, FollowDistance * 0.5f);
			return;
		}
	}

	// 플레이어 근처 → 랜덤 배회
	const float Now = GetWorld()->GetTimeSeconds();

	// 대기 중
	if (IsWanderIdle)
	{
		if (Now >= WanderIdleEndTime)
		{
			IsWanderIdle = false;
			ChooseNewWanderPoint();
		}
		return;
	}

	// 배회 지점이 없으면 새로 선택
	if (!IsWandering)
	{
		ChooseNewWanderPoint();
		return;
	}

	// 배회 지점 도달 체크
	const float DistToWander = FVector::Dist(OwnerEve->GetActorLocation(), WanderTarget);
	if (DistToWander < 50.f)
	{
		StopMovement();
		IsWanderIdle = true;
		WanderIdleEndTime = Now + FMath::RandRange(WanderIdleTime * 0.5f, WanderIdleTime * 1.5f);
	}
	else
	{
		MoveToLocation(WanderTarget, 30.f);
	}
}

void AMEveAIController::ChooseNewWanderPoint()
{
	// 플레이어 위치 중심으로 배회
	FVector Origin = OwnerEve->GetActorLocation();
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		Origin = PlayerPawn->GetActorLocation();
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation Result;
		if (NavSys->GetRandomReachablePointInRadius(Origin, WanderRadius, Result))
		{
			WanderTarget = Result.Location;
			IsWandering = true;
			MoveToLocation(WanderTarget, 30.f);
			return;
		}
	}

	// NavMesh 없을 경우 단순 랜덤
	WanderTarget = Origin + FVector(
		FMath::RandRange(-WanderRadius, WanderRadius),
		FMath::RandRange(-WanderRadius, WanderRadius),
		0.f);
	IsWandering = true;
	MoveToLocation(WanderTarget, 30.f);
}
