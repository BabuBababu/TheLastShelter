// Copyright TheLastShelter. All Rights Reserved.

#include "MOrdoAIController.h"
#include "MOrdoCharacter.h"
#include "MPlayerCharacter.h"
#include "MEveCharacter.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

AMOrdoAIController::AMOrdoAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMOrdoAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AMOrdoAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	OwnerOrdo = Cast<AMOrdoCharacter>(InPawn);
	SpawnLocation = InPawn->GetActorLocation();
	ChooseNewPatrolPoint();
}

void AMOrdoAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!OwnerOrdo || OwnerOrdo->IsDead()) return;

	// 1) 적(플레이어/Eve) 감지 시 → 추적 & 공격
	if (DetectedTarget)
	{
		const bool isEnemy = DetectedTarget->IsA<AMPlayerCharacter>() || DetectedTarget->IsA<AMEveCharacter>();
		if (isEnemy)
		{
			// 사망한 타겟은 무시
			bool isTargetAlive = true;
			if (const AMEveCharacter* Eve = Cast<AMEveCharacter>(DetectedTarget))
			{
				isTargetAlive = !Eve->IsDead();
			}

			if (isTargetAlive)
			{
				const float DistToTarget = FVector::Dist(OwnerOrdo->GetActorLocation(), DetectedTarget->GetActorLocation());

				if (DistToTarget <= AttackRange)
				{
					StopMovement();
					const float Now = GetWorld()->GetTimeSeconds();
					if (Now - LastAttackTime >= AttackRate)
					{
						OwnerOrdo->PerformAttack(DetectedTarget);
						LastAttackTime = Now;
					}
				}
				else
				{
					MoveToActor(DetectedTarget, AttackRange * 0.8f);
				}
				Patrolling = false;
				return;
			}
		}
	}

	// 2) 적 없으면 → 순찰
	if (!Patrolling)
	{
		ChooseNewPatrolPoint();
		Patrolling = true;
	}

	const float DistToPatrol = FVector::Dist(OwnerOrdo->GetActorLocation(), PatrolTarget);
	if (DistToPatrol < 50.f)
	{
		ChooseNewPatrolPoint();
	}

	MoveToLocation(PatrolTarget, 30.f);
}

void AMOrdoAIController::ChooseNewPatrolPoint()
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation Result;
		if (NavSys->GetRandomReachablePointInRadius(SpawnLocation, PatrolRadius, Result))
		{
			PatrolTarget = Result.Location;
			return;
		}
	}

	// NavMesh 없을 경우 단순 랜덤
	PatrolTarget = SpawnLocation + FVector(
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		0.f);
}
