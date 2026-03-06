// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MAttackTask.generated.h"

/**
 * UMAttackTask — 3단계 전투 태스크 (Attack / ForceAttack).
 *
 * 내부 상태 머신:
 *   Approaching → CombatEnter → CombatLoop ─→ (타겟 사망/소실) → CombatExit → 완료
 *                                      └──→ (사거리 이탈) → Approaching (재접근)
 *
 * ■ Attack 모드 (bForceMode=false):
 *   사거리 이탈 시 CombatLoop → Approaching 순환.
 *   타겟 사망/소실 시 CombatExit 후 태스크 완료.
 *
 * ■ ForceAttack 모드 (bForceMode=true):
 *   사거리 이탈해도 추격하면서 CombatLoop 유지.
 *   타겟 사망/소실 시에만 CombatExit.
 *
 * ■ 전투 진입 거리 (CombatEngageOffset):
 *   실제 전투 시작은 dist <= (AttackRange - CombatEngageOffset) 에서.
 *   사거리 경계가 아닌 더 가까운 거리에서 CombatEnter 진입.
 *
 * ■ 히스테리시스 (Oscillation 방지):
 *   사거리 '이탈'은 dist > AttackRange * RangeExitMultiplier 시 CombatExit.
 *   Normal 모드: 이탈 시 CombatExit → 태스크 완료 (Approaching 재순환 없음).
 *   → 경계에서 진동하지 않도록 버퍼 구간 확보.
 */
UCLASS()
class THELASTSHELTER_API UMAttackTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMAttackTask();

	// ============================================================
	// 설정 (팩토리에서 설정)
	// ============================================================

	/** Force 모드 — true이면 ForceAttack 동작 */
	bool bForceMode = false;

	/** 공격 사거리 */
	float AttackRange = 150.f;

	/** 공격 간격 (초) */
	float AttackRate = 1.5f;

	/** CombatEnter 애니메이션 지속 시간 (초) */
	float CombatEnterDuration = 0.6f;

	/** 전투 진입 오프셋 — AttackRange에서 이 값만큼 더 접근해야 CombatEnter 진입 */
	float CombatEngageOffset = 100.f;

	/** 사거리 이탈 히스테리시스 배율 (CombatLoop → CombatExit 전환 기준) */
	float RangeExitMultiplier = 1.3f;

	// ============================================================
	// 라이프사이클
	// ============================================================

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void EndTask(bool bWasCancelled) override;

private:
	/** 내부 전투 단계 */
	enum class ECombatPhase : uint8
	{
		Approaching,    // 타겟에게 이동 중
		CombatEnter,    // 전투 시작 애니메이션 재생
		CombatLoop,     // 전투 중 (사거리 내 공격 반복)
		CombatExit      // 전투 종료 (역재생) — 완료 후 태스크 종료
	};

	ECombatPhase CurrentPhase = ECombatPhase::Approaching;
	float PhaseStartTime = 0.f;
	float LastAttackTime = -999.f;

	/** CombatExit 완료 후 태스크 종료할지(타겟 사망) vs Approaching 복귀할지(거리 이탈) */
	bool bCombatExitEndsTask = false;

	// ---- 단계 전환 ----
	void TransitionTo(ECombatPhase NewPhase);

	// ---- 단계별 Tick ----
	void TickApproaching(float DeltaTime);
	void TickCombatEnter(float DeltaTime);
	void TickCombatLoop(float DeltaTime);
	void TickCombatExit(float DeltaTime);

	// ---- 유틸리티 ----

	/** 타겟 결정 — TargetActor 유효하면 사용, 아니면 컨트롤러에 위임 */
	AActor* ResolveTarget() const;

	/** 타겟 사망 체크 */
	bool IsTargetDead(AActor* Target) const;

	/** 캐릭터 타입(Eve/Ordo)에 맞는 전투 애니메이션 적용 */
	void ApplyCombatAnim(ECombatPhase Phase);

	/** 오너 캐릭터가 타겟을 바라보도록 */
	void FaceToTarget(AActor* Target);

	/** 오너 캐릭터의 공격 실행 */
	void PerformAttack(AActor* Target);

	/** 플립북 PlayRate 복원 (CombatExit 역재생 후) */
	void RestorePlayRate();
};
