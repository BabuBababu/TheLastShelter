// Copyright TheLastShelter. All Rights Reserved.

#include "MOrdoAIController.h"
#include "MOrdoCharacter.h"
#include "MPlayerCharacter.h"
#include "MEveCharacter.h"
#include "MAITaskComponent.h"
#include "NavigationSystem.h"
#include "Engine/Engine.h"

// ---- Task includes ----
#include "MAttackTask.h"
#include "MHoldPositionTask.h"
#include "MMoveTask.h"
#include "MRetreatTask.h"
#include "MAttackTargetTask.h"
#include "MKidnapTask.h"

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
// 태스크 팩토리 — EMTaskType → UMBaseTask* 생성
// ============================================================

UMBaseTask* AMOrdoAIController::CreateTaskForType(EMTaskType TaskType, AActor* Target, const FVector& Location)
{
	switch (TaskType)
	{
	// ---- 전투: Attack / ForceAttack ----
	case EMTaskType::Attack:
	case EMTaskType::ForceAttack:
	{
		UMAttackTask* task = NewObject<UMAttackTask>(this);
		task->TaskType = TaskType;
		task->bForceMode = (TaskType == EMTaskType::ForceAttack);
		task->AttackRange = AttackRange;
		task->AttackRate = AttackRate;
		task->CombatEnterDuration = CombatEnterDuration;
		task->CombatEngageOffset = CombatEngageOffset;
		task->TargetActor = Target;
		return task;
	}

	// ---- 전투: HoldPosition ----
	case EMTaskType::HoldPosition:
	{
		UMHoldPositionTask* task = NewObject<UMHoldPositionTask>(this);
		task->AttackRange = AttackRange;
		task->AttackRate = AttackRate;
		task->TargetActor = Target;
		return task;
	}

	// ---- 이동: ForceMove ----
	case EMTaskType::ForceMove:
	{
		UMMoveTask* task = NewObject<UMMoveTask>(this);
		task->TaskType = TaskType;
		task->TargetActor = Target;
		task->TargetLocation = Location;
		return task;
	}

	// ---- 전투: Retreat ----
	case EMTaskType::Retreat:
	{
		UMRetreatTask* task = NewObject<UMRetreatTask>(this);
		task->TargetLocation = !Location.IsNearlyZero() ? Location : RetreatDestination;
		task->ArrivalDistance = RetreatArrivalDist;
		task->bDestroyOnArrival = true;
		return task;
	}

	// ---- 전투: DestroyStorage ----
	case EMTaskType::DestroyStorage:
	{
		UMAttackTargetTask* task = NewObject<UMAttackTargetTask>(this);
		task->TaskType = TaskType;
		task->AttackRange = AttackRange;
		task->AttackRate = AttackRate;
		task->TargetActor = Target;
		return task;
	}

	// ---- 전투: Kidnap ----
	case EMTaskType::Kidnap:
	{
		UMKidnapTask* task = NewObject<UMKidnapTask>(this);
		task->TargetActor = Target;
		task->RetreatDestination = RetreatDestination;
		return task;
	}

	// ---- 전투: AttackDefenseTower ----
	case EMTaskType::AttackDefenseTower:
	{
		UMAttackTargetTask* task = NewObject<UMAttackTargetTask>(this);
		task->TaskType = TaskType;
		task->AttackRange = AttackRange;
		task->AttackRate = AttackRate;
		task->bIgnoreDamage = true;
		task->TargetActor = Target;
		return task;
	}

	default:
		return Super::CreateTaskForType(TaskType, Target, Location);
	}
}

// ============================================================
// 새 태스크 시작 콜백
// ============================================================

void AMOrdoAIController::OnNewTaskStarted(UMBaseTask* Task)
{
	Super::OnNewTaskStarted(Task);
	Patrolling = false;

	if (Task)
	{
		UE_LOG(LogTemp, Log, TEXT("[OrdoAI] Task Begin: %s"), *UEnum::GetValueAsString(Task->TaskType));
	}
}

// ============================================================
// Idle 행동 (태스크 없을 때) — 적 감지 + 순찰
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
				const float dist = FVector::Dist(OwnerOrdo->GetActorLocation(), DetectedTarget->GetActorLocation());
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Magenta,
					FString::Printf(TEXT("[Ordo] 적 감지! dist=%.0f, AtkRange=%.0f → Attack 태스크 생성"), dist, AttackRange));
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
// 우선 타겟 결정
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
