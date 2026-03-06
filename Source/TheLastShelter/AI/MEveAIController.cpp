// Copyright TheLastShelter. All Rights Reserved.

#include "MEveAIController.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "MAITaskComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Engine/Engine.h"
#include "ActorComponent/MStatComponent.h"
#include "Manager/MProjectileManager.h"

// ---- Task includes ----
#include "MAttackTask.h"
#include "MHoldPositionTask.h"
#include "MMoveTask.h"
#include "MIdleTask.h"
#include "MTimerActionTask.h"
#include "MCarryTask.h"
#include "MRetreatTask.h"

AMEveAIController::AMEveAIController()
{
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

// ============================================================
// 공격 간격 동적 계산
// ============================================================

float AMEveAIController::ComputeAttackRate() const
{
	// 맨유얼 오버라이드가 설정되어 있으면 즉시 사용
	if (AttackRateOverride > 0.f)
	{
		return AttackRateOverride;
	}

	if (!OwnerEve || !OwnerEve->StatComp)
	{
		// Eve 또는 StatComp가 없으면 기본값 (4발/초)
		return 1.f / 4.f;
	}

	// 기본 AttackSpeed (주로 스탯 컴포넌트의 보너스가 반영된 최종값)
	float attackSpeed = OwnerEve->StatComp->GetEffectiveStat().AttackSpeed;

	// 히든스탯 보너스
	attackSpeed += UMProjectileManager::GetHiddenAttackSpeedBonus(OwnerEve->HiddenStats);

	// 간격 변환: 1 / 초당발사횟수
	return 1.f / FMath::Max(attackSpeed, 0.1f);
}

// ============================================================
// 태스크 팩토리 — EMTaskType → UMBaseTask* 생성
// ============================================================

UMBaseTask* AMEveAIController::CreateTaskForType(EMTaskType TaskType, AActor* Target, const FVector& Location)
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
		task->AttackRate = ComputeAttackRate();
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
		task->AttackRate = ComputeAttackRate();
		task->TargetActor = Target;
		return task;
	}

	// ---- 이동: Move / ForceMove ----
	case EMTaskType::Move:
	case EMTaskType::ForceMove:
	{
		UMMoveTask* task = NewObject<UMMoveTask>(this);
		task->TaskType = TaskType;
		task->TargetActor = Target;
		task->TargetLocation = Location;
		return task;
	}

	// ---- 생활: Idle ----
	case EMTaskType::Idle:
	{
		UMIdleTask* task = NewObject<UMIdleTask>(this);
		// 플레이어 위치를 배회 중심으로
		if (APawn* playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			task->WanderOrigin = playerPawn->GetActorLocation();
		}
		else if (OwnerEve)
		{
			task->WanderOrigin = OwnerEve->GetActorLocation();
		}
		task->WanderRadius = WanderRadius;
		task->WanderIdleTime = WanderIdleTime;
		task->TotalDuration = WanderIdleTime * 3.f;
		return task;
	}

	// ---- 생활: Carry / ForceCarry ----
	case EMTaskType::Carry:
	case EMTaskType::ForceCarry:
	{
		UMCarryTask* task = NewObject<UMCarryTask>(this);
		task->TaskType = TaskType;
		task->TargetActor = Target;
		return task;
	}

	// ---- 생활: Rest / ForceRest ----
	case EMTaskType::Rest:
	case EMTaskType::ForceRest:
	{
		UMTimerActionTask* task = NewObject<UMTimerActionTask>(this);
		task->TaskType = TaskType;
		task->Duration = RestDuration;
		task->EveAnimState = EMEveAnimState::SitRest;
		return task;
	}

	// ---- 생활: Sex / ForceSex ----
	case EMTaskType::Sex:
	case EMTaskType::ForceSex:
	{
		UMTimerActionTask* task = NewObject<UMTimerActionTask>(this);
		task->TaskType = TaskType;
		task->Duration = SexDuration;
		task->EveAnimState = EMEveAnimState::Sex_Missionary;
		task->TargetActor = Target;
		return task;
	}

	// ---- 생활: Masturbation / ForceMasturbation ----
	case EMTaskType::Masturbation:
	case EMTaskType::ForceMasturbation:
	{
		UMTimerActionTask* task = NewObject<UMTimerActionTask>(this);
		task->TaskType = TaskType;
		task->Duration = MasturbationDuration;
		task->EveAnimState = EMEveAnimState::Sex_Masturbation;
		return task;
	}

	// ---- 전투: RetreatToShelter ----
	case EMTaskType::RetreatToShelter:
	{
		UMRetreatTask* task = NewObject<UMRetreatTask>(this);
		task->TaskType = TaskType;
		// TODO: 숙소 위치를 ShelterValueManager에서 가져오기
		task->TargetLocation = Location.IsNearlyZero() ? FVector::ZeroVector : Location;
		task->bDestroyOnArrival = false;
		return task;
	}

	// ---- 전투: UseSkill ----
	case EMTaskType::UseSkill:
	{
		// TODO: 스킬 시스템 연동
		UE_LOG(LogTemp, Log, TEXT("[EveAI] UseSkill: 미구현 스텁"));
		return nullptr;
	}

	default:
		return Super::CreateTaskForType(TaskType, Target, Location);
	}
}

// ============================================================
// 새 태스크 시작 콜백
// ============================================================

void AMEveAIController::OnNewTaskStarted(UMBaseTask* Task)
{
	Super::OnNewTaskStarted(Task);

	// 배회 상태 리셋
	IsWandering = false;
	IsWanderIdle = false;

	if (Task)
	{
		UE_LOG(LogTemp, Log, TEXT("[EveAI] Task Begin: %s"), *UEnum::GetValueAsString(Task->TaskType));
	}
}

// ============================================================
// Idle 행동 (태스크 없을 때)
// ============================================================

void AMEveAIController::ExecuteIdleBehavior(float DeltaTime)
{
	if (!OwnerEve || OwnerEve->IsDead()) return;

	// 적 감지 시 자동 전투 태스크 생성
	if (DetectedTarget && DetectedTarget->IsA<AMOrdoCharacter>())
	{
		AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(DetectedTarget);
		if (ordo && !ordo->IsDead())
		{
			const float dist = FVector::Dist(OwnerEve->GetActorLocation(), DetectedTarget->GetActorLocation());
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Magenta,
				FString::Printf(TEXT("[Eve] 적 감지! dist=%.0f, AtkRange=%.0f → Attack 태스크 생성"), dist, AttackRange));
			RequestTask(EMTaskType::Attack, EMTaskCategory::Combat, DetectedTarget);
			return;
		}
	}

	// 플레이어 추적
	APawn* playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (playerPawn)
	{
		const float distToPlayer = FVector::Dist(OwnerEve->GetActorLocation(), playerPawn->GetActorLocation());
		if (distToPlayer > FollowDistance)
		{
			IsWandering = false;
			IsWanderIdle = false;
			MoveToActor(playerPawn, FollowDistance * 0.5f);
			return;
		}
	}

	// 플레이어 근처 → 랜덤 배회
	const float now = GetWorld()->GetTimeSeconds();

	if (IsWanderIdle)
	{
		if (now >= WanderIdleEndTime)
		{
			IsWanderIdle = false;
			ChooseNewWanderPoint();
		}
		return;
	}

	if (!IsWandering)
	{
		ChooseNewWanderPoint();
		return;
	}

	const float distToWander = FVector::Dist(OwnerEve->GetActorLocation(), WanderTarget);
	if (distToWander < 50.f)
	{
		StopMovement();
		IsWanderIdle = true;
		WanderIdleEndTime = now + FMath::RandRange(WanderIdleTime * 0.5f, WanderIdleTime * 1.5f);
	}
	else
	{
		MoveToLocation(WanderTarget, 30.f);
	}
}

// ============================================================
// 우선 타겟 결정
// ============================================================

AActor* AMEveAIController::ResolveAttackTarget() const
{
	// 나를 공격한 대상이 시야 내이면 우선
	if (LastAttacker.IsValid() && !LastAttacker->IsActorBeingDestroyed())
	{
		if (const AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(LastAttacker.Get()))
		{
			if (!ordo->IsDead())
			{
				const float dist = FVector::Dist(OwnerEve->GetActorLocation(), LastAttacker->GetActorLocation());
				if (dist <= SightRadius)
				{
					return LastAttacker.Get();
				}
			}
		}
	}

	// 감지된 가장 가까운 적
	if (DetectedTarget && DetectedTarget->IsA<AMOrdoCharacter>())
	{
		const AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(DetectedTarget);
		if (ordo && !ordo->IsDead())
		{
			return DetectedTarget;
		}
	}

	return nullptr;
}

// ============================================================
// 배회 헬퍼
// ============================================================

void AMEveAIController::ChooseNewWanderPoint()
{
	FVector origin = OwnerEve ? OwnerEve->GetActorLocation() : FVector::ZeroVector;
	if (APawn* playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		origin = playerPawn->GetActorLocation();
	}

	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (navSys)
	{
		FNavLocation result;
		if (navSys->GetRandomReachablePointInRadius(origin, WanderRadius, result))
		{
			WanderTarget = result.Location;
			IsWandering = true;
			MoveToLocation(WanderTarget, 30.f);
			return;
		}
	}

	WanderTarget = origin + FVector(
		FMath::RandRange(-WanderRadius, WanderRadius),
		FMath::RandRange(-WanderRadius, WanderRadius),
		0.f);
	IsWandering = true;
	MoveToLocation(WanderTarget, 30.f);
}
