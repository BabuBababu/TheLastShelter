// Copyright TheLastShelter. All Rights Reserved.

#include "MEveAIController.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "MPlayerCharacter.h"
#include "MAITaskComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

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
// 태스크 시작 콜백
// ============================================================

void AMEveAIController::OnTaskBegin(const FMAITask& Task)
{
	TaskActionStartTime = GetWorld()->GetTimeSeconds();
	IsWandering = false;
	IsWanderIdle = false;

	UE_LOG(LogTemp, Log, TEXT("[EveAI] Task Begin: %s"), *UEnum::GetValueAsString(Task.TaskType));
}

// ============================================================
// 태스크 디스패치
// ============================================================

void AMEveAIController::ExecuteTask(float DeltaTime, const FMAITask& Task)
{
	if (!OwnerEve || OwnerEve->IsDead()) return;

	switch (Task.TaskType)
	{
	// ---- 전투 ----
	case EMTaskType::Attack:			ExecuteAttack(DeltaTime, Task); break;
	case EMTaskType::ForceAttack:		ExecuteForceAttack(DeltaTime, Task); break;
	case EMTaskType::HoldPosition:		ExecuteHoldPosition(DeltaTime); break;
	case EMTaskType::ForceMove:			ExecuteForceMove(DeltaTime, Task); break;
	case EMTaskType::RetreatToShelter:	ExecuteRetreatToShelter(DeltaTime); break;
	case EMTaskType::UseSkill:			ExecuteUseSkill(DeltaTime, Task); break;

	// ---- 생활 ----
	case EMTaskType::Idle:				ExecuteIdle(DeltaTime); break;
	case EMTaskType::Move:				ExecuteMove(DeltaTime, Task); break;
	case EMTaskType::Carry:
	case EMTaskType::ForceCarry:		ExecuteCarry(DeltaTime, Task); break;
	case EMTaskType::Rest:
	case EMTaskType::ForceRest:			ExecuteRest(DeltaTime); break;
	case EMTaskType::Sex:
	case EMTaskType::ForceSex:			ExecuteSex(DeltaTime, Task); break;
	case EMTaskType::Masturbation:
	case EMTaskType::ForceMasturbation:	ExecuteMasturbation(DeltaTime); break;

	default:
		// 알 수 없는 태스크 → 완료 처리
		if (TaskComp) TaskComp->CompleteCurrentTask();
		break;
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
// 전투 태스크 구현
// ============================================================

AActor* AMEveAIController::ResolveAttackTarget() const
{
	// 나를 공격한 대상이 사거리 내이면 우선
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

void AMEveAIController::ExecuteAttack(float DeltaTime, const FMAITask& Task)
{
	AActor* target = Task.TargetActor.IsValid() ? Task.TargetActor.Get() : ResolveAttackTarget();

	if (!target)
	{
		// 타겟 없음 → 태스크 완료
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	// 타겟 사망 체크
	if (const AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(target))
	{
		if (ordo->IsDead())
		{
			if (TaskComp) TaskComp->CompleteCurrentTask();
			return;
		}
	}

	const float distToTarget = FVector::Dist(OwnerEve->GetActorLocation(), target->GetActorLocation());

	if (distToTarget <= AttackRange)
	{
		StopMovement();
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerEve->PerformAttack(target);
			LastAttackTime = now;
		}
	}
	else
	{
		MoveToActor(target, AttackRange * 0.8f);
	}
}

void AMEveAIController::ExecuteForceAttack(float DeltaTime, const FMAITask& Task)
{
	// ForceAttack은 사거리 밖이어도 반드시 이동 후 공격
	AActor* target = Task.TargetActor.IsValid() ? Task.TargetActor.Get() : ResolveAttackTarget();

	if (!target)
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	if (const AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(target))
	{
		if (ordo->IsDead())
		{
			if (TaskComp) TaskComp->CompleteCurrentTask();
			return;
		}
	}

	const float distToTarget = FVector::Dist(OwnerEve->GetActorLocation(), target->GetActorLocation());

	if (distToTarget <= AttackRange)
	{
		StopMovement();
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerEve->PerformAttack(target);
			LastAttackTime = now;
		}
	}
	else
	{
		// 거리 무시하고 무조건 접근
		MoveToActor(target, AttackRange * 0.5f);
	}
}

void AMEveAIController::ExecuteHoldPosition(float DeltaTime)
{
	// 위치 고수 — 이동하지 않고 제자리에서 사거리 내 적만 공격
	StopMovement();

	AActor* target = ResolveAttackTarget();
	if (!target) return;

	const float distToTarget = FVector::Dist(OwnerEve->GetActorLocation(), target->GetActorLocation());
	if (distToTarget <= AttackRange)
	{
		const float now = GetWorld()->GetTimeSeconds();
		if (now - LastAttackTime >= AttackRate)
		{
			OwnerEve->PerformAttack(target);
			LastAttackTime = now;
		}
	}
	// 사거리 밖이면 무시 (이동 안함)
}

void AMEveAIController::ExecuteForceMove(float DeltaTime, const FMAITask& Task)
{
	const FVector destination = Task.TargetLocation;
	const float dist = FVector::Dist(OwnerEve->GetActorLocation(), destination);

	if (dist < 50.f)
	{
		StopMovement();
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	MoveToLocation(destination, 30.f);
}

void AMEveAIController::ExecuteRetreatToShelter(float DeltaTime)
{
	// TODO: 숙소 위치를 ShelterValueManager 등에서 가져오기
	// 임시로 월드 원점 사용
	const FVector shelterLocation = FVector::ZeroVector;
	const float dist = FVector::Dist(OwnerEve->GetActorLocation(), shelterLocation);

	if (dist < 100.f)
	{
		StopMovement();
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	MoveToLocation(shelterLocation, 50.f);
}

void AMEveAIController::ExecuteUseSkill(float DeltaTime, const FMAITask& Task)
{
	// TODO: SkillIndex로 Eve 스킬 시스템 연동
	// 현재는 즉시 완료 처리
	UE_LOG(LogTemp, Log, TEXT("[EveAI] UseSkill: Slot %d"), Task.SkillIndex);
	if (TaskComp) TaskComp->CompleteCurrentTask();
}

// ============================================================
// 생활 태스크 구현
// ============================================================

void AMEveAIController::ExecuteIdle(float DeltaTime)
{
	// Idle 상태에서 멘탈 성향 기반 랜덤 행동
	// 높은 멘탈 → 주변 배회, 낮은 멘탈 → 가만히 있기
	const float now = GetWorld()->GetTimeSeconds();
	const float elapsed = now - TaskActionStartTime;

	// 일정 시간 후 태스크 완료 (다음 Idle이 자동 반복)
	if (elapsed > WanderIdleTime * 3.f)
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	// 배회 로직 재사용
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

void AMEveAIController::ExecuteMove(float DeltaTime, const FMAITask& Task)
{
	// 지정 위치로 이동 (ForceMove와 동일하지만 일반 우선순위)
	const FVector destination = Task.TargetActor.IsValid()
		? Task.TargetActor->GetActorLocation()
		: Task.TargetLocation;

	const float dist = FVector::Dist(OwnerEve->GetActorLocation(), destination);
	if (dist < 50.f)
	{
		StopMovement();
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	if (Task.TargetActor.IsValid())
	{
		MoveToActor(Task.TargetActor.Get(), 30.f);
	}
	else
	{
		MoveToLocation(destination, 30.f);
	}
}

void AMEveAIController::ExecuteCarry(float DeltaTime, const FMAITask& Task)
{
	// 운반: 대상 액터 위치로 이동 → 도착 시 운반 완료
	// TODO: 실제 아이템 픽업/운반 로직 연동
	if (!Task.TargetActor.IsValid())
	{
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	const float dist = FVector::Dist(OwnerEve->GetActorLocation(), Task.TargetActor->GetActorLocation());
	if (dist < 80.f)
	{
		StopMovement();
		OwnerEve->SetAnimState(EMEveAnimState::BoxCarry_Down);
		UE_LOG(LogTemp, Log, TEXT("[EveAI] Carry: Picked up target."));
		if (TaskComp) TaskComp->CompleteCurrentTask();
		return;
	}

	MoveToActor(Task.TargetActor.Get(), 50.f);
}

void AMEveAIController::ExecuteRest(float DeltaTime)
{
	StopMovement();
	OwnerEve->SetAnimState(EMEveAnimState::SitRest);

	const float elapsed = GetWorld()->GetTimeSeconds() - TaskActionStartTime;
	if (elapsed >= RestDuration)
	{
		UE_LOG(LogTemp, Log, TEXT("[EveAI] Rest complete."));
		if (TaskComp) TaskComp->CompleteCurrentTask();
	}
}

void AMEveAIController::ExecuteSex(float DeltaTime, const FMAITask& Task)
{
	StopMovement();
	// 애니메이션 상태 설정
	OwnerEve->SetAnimState(EMEveAnimState::Sex_Missionary);

	const float elapsed = GetWorld()->GetTimeSeconds() - TaskActionStartTime;
	if (elapsed >= SexDuration)
	{
		UE_LOG(LogTemp, Log, TEXT("[EveAI] Sex task complete."));
		if (TaskComp) TaskComp->CompleteCurrentTask();
	}
}

void AMEveAIController::ExecuteMasturbation(float DeltaTime)
{
	StopMovement();
	OwnerEve->SetAnimState(EMEveAnimState::Sex_Masturbation);

	const float elapsed = GetWorld()->GetTimeSeconds() - TaskActionStartTime;
	if (elapsed >= MasturbationDuration)
	{
		UE_LOG(LogTemp, Log, TEXT("[EveAI] Masturbation task complete."));
		if (TaskComp) TaskComp->CompleteCurrentTask();
	}
}

// ============================================================
// 배회 헬퍼
// ============================================================

void AMEveAIController::ChooseNewWanderPoint()
{
	FVector origin = OwnerEve->GetActorLocation();
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
