// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MAIControllerBase.h"
#include "MEveAIController.generated.h"

class AMEveCharacter;
class UMBaseTask;

/**
 * MEveAIController
 * Eve(미소녀) 전용 AI 컨트롤러. 플레이어 편.
 *
 * Command Pattern으로 리팩토링됨:
 * - 모든 행동은 UMBaseTask 서브클래스가 자체 state machine으로 구현
 * - 컨트롤러는 태스크 팩토리(CreateTaskForType)와 Idle 행동만 담당
 *
 * === 전투 태스크 ===
 *   Attack / ForceAttack – UMAttackTask
 *   HoldPosition         – UMHoldPositionTask
 *   ForceMove            – UMMoveTask
 *   RetreatToShelter     – UMRetreatTask
 *   UseSkill             – TODO
 *
 * === 생활 태스크 ===
 *   Idle                 – UMIdleTask
 *   Move                 – UMMoveTask
 *   Carry / ForceCarry   – UMCarryTask
 *   Rest / ForceRest     – UMTimerActionTask
 *   Sex / ForceSex       – UMTimerActionTask
 *   Masturbation / ForceMasturbation – UMTimerActionTask
 */
UCLASS()
class THELASTSHELTER_API AMEveAIController : public AMAIControllerBase
{
	GENERATED_BODY()

public:
	AMEveAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// ---- Command Pattern 인터페이스 ----
	virtual UMBaseTask* CreateTaskForType(EMTaskType TaskType, AActor* Target, const FVector& Location) override;
	virtual void ExecuteIdleBehavior(float DeltaTime) override;
	virtual void OnNewTaskStarted(UMBaseTask* Task) override;
	virtual AActor* ResolveAttackTarget() const override;

	/** 소유 Eve 캐싱 */
	UPROPERTY()
	TObjectPtr<AMEveCharacter> OwnerEve;

public:
	// ============================================================
	// 튜닝 파라미터
	// ============================================================

	/** 플레이어 추적 거리 (이 범위 밖이면 따라감) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float FollowDistance = 300.f;

	/** 전투 공격 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float AttackRange = 800.f;

	/**
	 * 공격 간격 오버라이드 (초). 0이면 Eve의 AttackSpeed 스탯으로 자동 계산.
	 * 디버깅/밸런싱 용 수동 고정값.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float AttackRateOverride = 0.f;

	/** 배회 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float WanderRadius = 250.f;

	/** 배회 대기 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float WanderIdleTime = 2.0f;

	/** 휴식 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float RestDuration = 5.0f;

	/** 성행위 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float SexDuration = 8.0f;

	/** 자위 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float MasturbationDuration = 6.0f;

	/** 전투 시작 애니메이션 지속 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float CombatEnterDuration = 0.6f;

	/** 전투 진입 오프셋 — AttackRange에서 이 값만큼 더 첑분히 접근해야 CombatEnter 진입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float CombatEngageOffset = 100.f;

private:
	/**
	 * Eve의 AttackSpeed 스탯 + 히든스탯 보너스로 공격 간격(초)을 계산.
	 * AttackRateOverride > 0이면 그 값을 즉시 반환.
	 */
	float ComputeAttackRate() const;

	// ---- Idle 배회 상태 ----
	FVector WanderTarget = FVector::ZeroVector;
	bool IsWandering = false;
	float WanderIdleEndTime = 0.f;
	bool IsWanderIdle = false;
	void ChooseNewWanderPoint();
};
