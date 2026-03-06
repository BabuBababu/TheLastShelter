// Copyright TheLastShelter. All Rights Reserved.

#include "MAttackTask.h"
#include "MAIControllerBase.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Engine/Engine.h"

UMAttackTask::UMAttackTask()
{
	TaskType = EMTaskType::Attack;
	Category = EMTaskCategory::Combat;
}

// ============================================================
// 라이프사이클
// ============================================================

void UMAttackTask::StartTask()
{
	Super::StartTask();
	LastAttackTime = -999.f;
	TransitionTo(ECombatPhase::Approaching);
}

void UMAttackTask::TickTask(float DeltaTime)
{
	// CombatExit는 타겟 유무와 관계없이 역재생 완료까지 진행
	if (CurrentPhase == ECombatPhase::CombatExit)
	{
		TickCombatExit(DeltaTime);
		return;
	}

	AActor* target = ResolveTarget();

	if (!target || IsTargetDead(target))
	{
		// 타겟 없음/사망 → 전투 중이면 CombatExit(태스크 종료), 접근 중이면 바로 종료
		if (CurrentPhase == ECombatPhase::CombatLoop || CurrentPhase == ECombatPhase::CombatEnter)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				FString::Printf(TEXT("[%s] 타겟 없음/사망 → CombatExit"),
					GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo")));
			bCombatExitEndsTask = true;
			TransitionTo(ECombatPhase::CombatExit);
		}
		else
		{
			FinishTask(true);
		}
		return;
	}

	switch (CurrentPhase)
	{
	case ECombatPhase::Approaching:   TickApproaching(DeltaTime); break;
	case ECombatPhase::CombatEnter:   TickCombatEnter(DeltaTime); break;
	case ECombatPhase::CombatLoop:    TickCombatLoop(DeltaTime); break;
	default: break;
	}
}

void UMAttackTask::EndTask(bool bWasCancelled)
{
	if (bWasCancelled)
	{
		RestorePlayRate();
		ApplyCombatAnim(ECombatPhase::Approaching); // GunIdle로 복원
	}
	StopOwnerMovement();
	Super::EndTask(bWasCancelled);
}

// ============================================================
// 단계 전환
// ============================================================

void UMAttackTask::TransitionTo(ECombatPhase NewPhase)
{
	// 동일 페이즈 재진입 방지
	if (CurrentPhase == NewPhase) return;

	const ECombatPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;
	PhaseStartTime = GetWorldTime();

	const FString ownerName = GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo");

	if (GEngine)
	{
		auto PhaseName = [](ECombatPhase p) -> const TCHAR*
		{
			switch (p)
			{
			case ECombatPhase::Approaching:  return TEXT("Approaching");
			case ECombatPhase::CombatEnter:  return TEXT("CombatEnter");
			case ECombatPhase::CombatLoop:   return TEXT("CombatLoop");
			case ECombatPhase::CombatExit:   return TEXT("CombatExit");
			default: return TEXT("Unknown");
			}
		};

		const FColor color =
			NewPhase == ECombatPhase::Approaching ? FColor::Silver :
			NewPhase == ECombatPhase::CombatEnter ? FColor::Green :
			NewPhase == ECombatPhase::CombatLoop  ? FColor::Cyan :
			FColor::Orange;

		GEngine->AddOnScreenDebugMessage(-1, 3.f, color,
			FString::Printf(TEXT("[%s] 단계전환: %s → %s"), *ownerName, PhaseName(OldPhase), PhaseName(NewPhase)));
	}

	switch (NewPhase)
	{
	case ECombatPhase::Approaching:
		ApplyCombatAnim(ECombatPhase::Approaching); // GunIdle
		break;

	case ECombatPhase::CombatEnter:
		StopOwnerMovement();
		ApplyCombatAnim(ECombatPhase::CombatEnter);
		break;

	case ECombatPhase::CombatLoop:
		ApplyCombatAnim(ECombatPhase::CombatLoop);
		break;

	case ECombatPhase::CombatExit:
		StopOwnerMovement();
		ApplyCombatAnim(ECombatPhase::CombatExit);
		break;
	}
}

// ============================================================
// 단계별 Tick
// ============================================================

void UMAttackTask::TickApproaching(float DeltaTime)
{
	AActor* target = ResolveTarget();
	if (!target) return;

	const float dist = GetDistanceTo(target);

	// 전투 진입 거리 = AttackRange - CombatEngageOffset (최소 30)
	const float engageDist = FMath::Max(AttackRange - CombatEngageOffset, 30.f);

	// 진입 거리 이내 → 항상 CombatEnter 진행 (Approaching 직후는 반드시 CombatEnter)
	if (dist <= engageDist)
	{
		StopOwnerMovement();
		FaceToTarget(target);
		TransitionTo(ECombatPhase::CombatEnter);
		return;
	}

	// 이동 중이 아니면 이동 요청
	if (!IsOwnerMoving())
	{
		MoveToTarget(target, engageDist * 0.5f);
	}
}

void UMAttackTask::TickCombatEnter(float DeltaTime)
{
	AActor* target = ResolveTarget();
	if (!target) return;

	const float dist = GetDistanceTo(target);
	const float elapsed = GetWorldTime() - PhaseStartTime;

	// Normal 모드: 사거리 크게 이탈 시 CombatExit → 태스크 종료
	if (!bForceMode && dist > AttackRange * RangeExitMultiplier)
	{
		TransitionTo(ECombatPhase::CombatExit);
		return;
	}

	// Force 모드: 사거리 밖이면 추격하면서 CombatEnter 유지
	if (bForceMode && dist > AttackRange)
	{
		if (!IsOwnerMoving()) MoveToTarget(target, AttackRange * 0.3f);
	}
	else
	{
		StopOwnerMovement();
	}

	FaceToTarget(target);

	// CombatEnter 애니메이션 완료 → CombatLoop
	// 비루핑 플립북 재생 완료 감지 (CombatEnterDuration은 폴백)
	bool bAnimFinished = false;
	if (AMEveCharacter* eve = GetOwnerEve())
	{
		if (eve->GetSprite()) bAnimFinished = !eve->GetSprite()->IsPlaying();
	}
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
	{
		if (ordo->GetSprite()) bAnimFinished = !ordo->GetSprite()->IsPlaying();
	}

	if (bAnimFinished || elapsed >= CombatEnterDuration)
	{
		TransitionTo(ECombatPhase::CombatLoop);
	}
}

void UMAttackTask::TickCombatLoop(float DeltaTime)
{
	AActor* target = ResolveTarget();
	if (!target) return;

	const float dist = GetDistanceTo(target);

	// 사거리 이탈 체크 (히스테리시스 적용)
	if (dist > AttackRange * RangeExitMultiplier)
	{
		if (bForceMode)
		{
			// ForceAttack: CombatLoop 유지하면서 추격
			const float engageDist = FMath::Max(AttackRange - CombatEngageOffset, 30.f);
			if (!IsOwnerMoving()) MoveToTarget(target, engageDist * 0.5f);
			FaceToTarget(target);
			return;
		}
		else
		{
			// Normal Attack: CombatExit 역재생 후 Approaching 복귀 (태스크 유지)
			bCombatExitEndsTask = false;
			TransitionTo(ECombatPhase::CombatExit);
			return;
		}
	}

	// 사거리 내 — 정지 후 공격
	StopOwnerMovement();
	FaceToTarget(target);

	const float now = GetWorldTime();
	if (now - LastAttackTime >= AttackRate)
	{
		PerformAttack(target);
		LastAttackTime = now;
	}
}

void UMAttackTask::TickCombatExit(float DeltaTime)
{
	const float elapsed = GetWorldTime() - PhaseStartTime;

	if (elapsed >= CombatEnterDuration)
	{
		RestorePlayRate();

		if (bCombatExitEndsTask)
		{
			// 타겟 사망/소실 → GunIdle 복원 후 태스크 종료
			ApplyCombatAnim(ECombatPhase::Approaching);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White,
				FString::Printf(TEXT("[%s] CombatExit 완료 → 태스크 종료"),
					GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo")));
			FinishTask(true);
		}
		else
		{
			// 거리 이탈 → Approaching 복귀 (재추격 → CombatEnter → CombatLoop 순환)
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White,
				FString::Printf(TEXT("[%s] CombatExit 완료 → Approaching 복귀 (재추격)"),
					GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo")));
			TransitionTo(ECombatPhase::Approaching);
		}
	}
}

// ============================================================
// 유틸리티
// ============================================================

AActor* UMAttackTask::ResolveTarget() const
{
	// 설정된 타겟이 유효하면 사용
	if (TargetActor.IsValid() && !IsTargetDead(TargetActor.Get()))
	{
		return TargetActor.Get();
	}

	// 컨트롤러에게 타겟 결정 위임
	if (AMAIControllerBase* ctrl = GetBaseController())
	{
		return ctrl->ResolveAttackTarget();
	}

	return nullptr;
}

bool UMAttackTask::IsTargetDead(AActor* Target) const
{
	if (!Target || Target->IsActorBeingDestroyed()) return true;

	if (const AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(Target))
		return ordo->IsDead();
	if (const AMEveCharacter* eve = Cast<AMEveCharacter>(Target))
		return eve->IsDead();

	return false;
}

void UMAttackTask::ApplyCombatAnim(ECombatPhase Phase)
{
	if (AMEveCharacter* eve = GetOwnerEve())
	{
		switch (Phase)
		{
		case ECombatPhase::Approaching:
			eve->SetAnimState(EMEveAnimState::GunIdle); break;
		case ECombatPhase::CombatEnter:
			eve->SetAnimState(EMEveAnimState::CombatEnter);
			// CombatEnter는 1회만 재생 (루프 방지)
			if (eve->GetSprite()) eve->GetSprite()->SetLooping(false);
			break;
		case ECombatPhase::CombatLoop:
			// 루프 복원 후 CombatLoop 재생
			if (eve->GetSprite()) eve->GetSprite()->SetLooping(true);
			eve->SetAnimState(EMEveAnimState::CombatLoop);
			break;
		case ECombatPhase::CombatExit:
			eve->PlayCombatExitAnim(); break;
		}
	}
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
	{
		switch (Phase)
		{
		case ECombatPhase::Approaching:
			ordo->SetAnimState(EMOrdoAnimState::GunIdle); break;
		case ECombatPhase::CombatEnter:
			ordo->SetAnimState(EMOrdoAnimState::CombatEnter);
			if (ordo->GetSprite()) ordo->GetSprite()->SetLooping(false);
			break;
		case ECombatPhase::CombatLoop:
			if (ordo->GetSprite()) ordo->GetSprite()->SetLooping(true);
			ordo->SetAnimState(EMOrdoAnimState::CombatLoop);
			break;
		case ECombatPhase::CombatExit:
			ordo->PlayCombatExitAnim(); break;
		}
	}
}

void UMAttackTask::FaceToTarget(AActor* Target)
{
	if (AMEveCharacter* eve = GetOwnerEve())
		eve->FaceTarget(Target);
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
		ordo->FaceTarget(Target);
}

void UMAttackTask::PerformAttack(AActor* Target)
{
	if (AMEveCharacter* eve = GetOwnerEve())
		eve->PerformAttack(Target);
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
		ordo->PerformAttack(Target);
}

void UMAttackTask::RestorePlayRate()
{
	if (AMEveCharacter* eve = GetOwnerEve())
	{
		if (eve->GetSprite()) eve->GetSprite()->SetPlayRate(1.0f);
	}
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
	{
		if (ordo->GetSprite()) ordo->GetSprite()->SetPlayRate(1.0f);
	}
}
