// Copyright TheLastShelter. All Rights Reserved.

#include "MOrdoAIController.h"
#include "MOrdoCharacter.h"
#include "MPlayerCharacter.h"
#include "MEveCharacter.h"
#include "MAITaskComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

AMOrdoAIController::AMOrdoAIController()
{
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

	// 퇴각 기본 목적지: 스폰 위치 (레벨에서 오버라이드 가능)
	RetreatDestination = SpawnLocation;

	ChooseNewPatrolPoint();
}

// ============================================================
// 태스크 시작 콜백
// ============================================================

void AMOrdoAIController::OnTaskBegin(const FMAITask& Task)
{
	Patrolling = false;
	UE_LOG(LogTemp, Log, TEXT("[OrdoAI] Task Begin: %s"), *UEnum::GetValueAsString(Task.TaskType));
}

// ============================================================
// 태스크 디스패치
// ============================================================

void AMOrdoAIController::ExecuteTask(float DeltaTime, const FMAITask& Task)
{
	if (!OwnerOrdo || OwnerOrdo->IsDead()) return;

	switch (Task.TaskType)
	{
	case EMTaskType::Attack:				ExecuteAttack(DeltaTime, Task); break;
	case EMTaskType::ForceAttack:			ExecuteForceAttack(DeltaTime, Task); break;
	case EMTaskType::HoldPosition:			ExecuteHoldPosition(DeltaTime); break;
	case EMTaskType::ForceMove:				ExecuteForceMove(DeltaTime, Task); break;
	case EMTaskType::Retreat:				ExecuteRetreat(DeltaTime, Task); break;
	case EMTaskType::DestroyStorage:		ExecuteDestroyStorage(DeltaTime, Task); break;
	case EMTaskType::Kidnap:				ExecuteKidnap(DeltaTime, Task); break;
	case EMTaskType::AttackDefenseTower:	ExecuteAttackDefenseTower(DeltaTime, Task); break;

	default:
		if (TaskComp) TaskComp->CompleteCurrentTask();
		break;
	}
}

// ============================================================
// Idle 행동 (태스크 없을 때)
// ============================================================

void AMOrdoAIController::ExecuteIdleBehavior(float DeltaTime)
{
	if (!OwnerOrdo || OwnerOrdo->IsDead()) return;

	// 적 감지 시 자동 공격 태스크
	if (DetectedTarget)
	{
		const bool isEnemy = DetectedTarget->IsA<AMPlayerCharacter>() || DetectedTarget->IsA<AMEveCharacter>();
		if (isEnemy)
		{
			bool isTargetAlive = true;
			if (const AMEveCharacter* eve = Cast<AMEveCharacter>(DetectedTarget))
			{
				isTargetAlive = !eve->IsDead();
			}

			if (isTargetAlive)
			{
				RequestTask(EMTaskType::Attack, EMTaskCategory::Combat, DetectedTarget);
				return;
			}
		}
	}

	// 순찰
	if (!Patrolling)
	{
		ChooseNewPatrolPoint();
		Patrolling = true;
	}

	const float distToPatrol = FVector::Dist(OwnerOrdo->GetActorLocation(), PatrolTarget);
	if (distToPatrol < 50.f)
	{
		ChooseNewPatrolPoint();
	}

	MoveToLocation(PatrolTarget, 30.f);
}

// ============================================================
// 전투 태스크 구현
// ============================================================

AActor* AMOrdoAIController::ResolveAttackTarget() const
{
	// 나를 공격한 대상 우선
	if (LastAttacker.IsValid() && !LastAttacker->IsActorBeingDestroyed())
	{
		const bool isEnemy = LastAttacker->IsA<AMPlayerCharacter>() || LastAttacker->IsA<AMEveCharacter>();
		if (isEnemy)
		{
			bool isAlive = true;
			if (const AMEveCharacter* eve = Cast<AMEveCharacter>(LastAttacker.Get()))
			{
				isAlive = !eve->IsDead();
			}
			if (isAlive)
			{
				const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), LastAttacker->GetActorLocation());
				if (dist <= SightRadius)
				{
					return LastAttacker.Get();
				}
			}
		}
	}

	// 감지된 가장 가까운 적
	if (DetectedTarget)
	{
		const bool isEnemy = DetectedTarget->IsA<AMPlayerCharacter>() || DetectedTarget->IsA<AMEveCharacter>();
		if (isEnemy)
		{
			bool isAlive = true;
			if (const AMEveCharacter* eve = Cast<AMEveCharacter>(DetectedTarget))
			{
				isAlive = !eve->IsDead();
			}
			if (isAlive) return DetectedTarget;
		}
	}

	return nullptr;
}

void AMOrdoAIController::ExecuteAttack(float DeltaTime, const FMAITask& Task)
{
	AActor* target = Task.TargetActor.IsValid() ? Task.TargetActor.Get() : ResolveAttackTarget();

	if (!target)
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	// 타겟 사망 체크
	if (const AMEveCharacter* eve = Cast<AMEveCharacter>(target))
	{
		if (eve->IsDead()) { if (TaskComp) TaskComp->CompleteCurrentTask(); return; }
	}

	const float distToTarget = FVector::Dist(OwnerOrdo->GetActorLocation(), target->GetActorLocation());

	if (distToTarget <= AttackRange)
	{
		StopMovement();
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerOrdo->PerformAttack(target);
			LastAttackTime = now;
		}
	}
	else
	{
		MoveToActor(target, AttackRange * 0.8f);
	}
}

void AMOrdoAIController::ExecuteForceAttack(float DeltaTime, const FMAITask& Task)
{
	AActor* target = Task.TargetActor.IsValid() ? Task.TargetActor.Get() : ResolveAttackTarget();

	if (!target)
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	if (const AMEveCharacter* eve = Cast<AMEveCharacter>(target))
	{
		if (eve->IsDead()) { if (TaskComp) TaskComp->CompleteCurrentTask(); return; }
	}

	const float distToTarget = FVector::Dist(OwnerOrdo->GetActorLocation(), target->GetActorLocation());

	if (distToTarget <= AttackRange)
	{
		StopMovement();
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerOrdo->PerformAttack(target);
			LastAttackTime = now;
		}
	}
	else
	{
		MoveToActor(target, AttackRange * 0.5f);
	}
}

void AMOrdoAIController::ExecuteHoldPosition(float DeltaTime)
{
	StopMovement();

	AActor* target = ResolveAttackTarget();
	if (!target) return;

	const float distToTarget = FVector::Dist(OwnerOrdo->GetActorLocation(), target->GetActorLocation());
	if (distToTarget <= AttackRange)
	{
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerOrdo->PerformAttack(target);
			LastAttackTime = now;
		}
	}
}

void AMOrdoAIController::ExecuteForceMove(float DeltaTime, const FMAITask& Task)
{
	const FVector destination = Task.TargetLocation;
	const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), destination);

	if (dist < 50.f)
	{
		StopMovement();
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	MoveToLocation(destination, 30.f);
}

void AMOrdoAIController::ExecuteRetreat(float DeltaTime, const FMAITask& Task)
{
	// 레벨 지정 스팟(TargetLocation)이 있으면 그곳으로, 아니면 RetreatDestination(스폰 위치)
	const FVector destination = !Task.TargetLocation.IsNearlyZero()
		? Task.TargetLocation
		: RetreatDestination;

	const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), destination);

	if (dist < RetreatArrivalDist)
	{
		StopMovement();
		UE_LOG(LogTemp, Log, TEXT("[OrdoAI] Retreat arrived — despawning."));
		// 디스폰
		if (OwnerOrdo)
		{
			OwnerOrdo->Destroy();
		}
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	MoveToLocation(destination, 50.f);
}

void AMOrdoAIController::ExecuteDestroyStorage(float DeltaTime, const FMAITask& Task)
{
	// TODO: Storage 액터 참조 시스템 연동
	// 현재는 TargetActor로 전달받아 접근 후 공격
	if (!Task.TargetActor.IsValid())
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), Task.TargetActor->GetActorLocation());
	if (dist <= AttackRange)
	{
		StopMovement();
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerOrdo->PerformAttack(Task.TargetActor.Get());
			LastAttackTime = now;
		}
	}
	else
	{
		MoveToActor(Task.TargetActor.Get(), AttackRange * 0.8f);
	}
}

void AMOrdoAIController::ExecuteKidnap(float DeltaTime, const FMAITask& Task)
{
	// 1단계: 기절한 Eve에게 접근
	// 2단계: Eve를 엎고 퇴각 (Retreat)
	AMEveCharacter* eve = Cast<AMEveCharacter>(Task.TargetActor.Get());
	if (!eve || !eve->IsDead()) // IsDead = 기절/무력화 상태로 간주
	{
		// 타겟이 없거나 아직 기절하지 않음
		if (!eve)
		{
			if (TaskComp) TaskComp->CompleteCurrentTask();
			return;
		}
		// 기절하지 않은 Eve → 먼저 공격
		ExecuteAttack(DeltaTime, Task);
		return;
	}

	const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), eve->GetActorLocation());

	if (dist < 80.f)
	{
		// Eve를 "납치" → 퇴각으로 전환
		KidnapTarget = eve;
		UE_LOG(LogTemp, Log, TEXT("[OrdoAI] Kidnapping Eve: %s"), *eve->GetName());

		// Eve를 Ordo에 부착 (간단 구현)
		eve->AttachToActor(OwnerOrdo, FAttachmentTransformRules::KeepRelativeTransform);

		// 퇴각 태스크로 전환
		if (TaskComp)
		{
			TaskComp->CompleteCurrentTask();
			FMAITask retreatTask = FMAITask::MakeWithLocation(
				EMTaskType::Retreat, EMTaskCategory::Combat, RetreatDestination, EMTaskPriority::High);
			TaskComp->EnqueueTask(retreatTask);
		}
		return;
	}

	MoveToActor(eve, 50.f);
}

void AMOrdoAIController::ExecuteAttackDefenseTower(float DeltaTime, const FMAITask& Task)
{
	// 방어 타워만 집중 공격 — 다른 대상의 공격 무시
	if (!Task.TargetActor.IsValid())
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), Task.TargetActor->GetActorLocation());

	if (dist <= AttackRange)
	{
		StopMovement();
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerOrdo->PerformAttack(Task.TargetActor.Get());
			LastAttackTime = now;
		}
	}
	else
	{
		// 데미지 무시하고 타워로 직행
		MoveToActor(Task.TargetActor.Get(), AttackRange * 0.5f);
	}
}

// ============================================================
// 순찰 헬퍼
// ============================================================

void AMOrdoAIController::ChooseNewPatrolPoint()
{
	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (navSys)
	{
		FNavLocation result;
		if (navSys->GetRandomReachablePointInRadius(SpawnLocation, PatrolRadius, result))
		{
			PatrolTarget = result.Location;
			return;
		}
	}

	PatrolTarget = SpawnLocation + FVector(
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		FMath::RandRange(-PatrolRadius, PatrolRadius),
		0.f);
}
