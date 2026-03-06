// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MAIControllerBase.h"
#include "MOrdoAIController.generated.h"

class AMOrdoCharacter;
class AMEveCharacter;
class UMBaseTask;

/**
 * MOrdoAIController
 * Ordo(적) 전용 AI 컨트롤러.
 *
 * Command Pattern으로 리팩토링됨:
 * - 모든 행동은 UMBaseTask 서브클래스가 자체 state machine으로 구현
 * - 컨트롤러는 태스크 팩토리(CreateTaskForType)와 Idle 순찰만 관장
 *
 * === 전투 태스크 ===
 *   Attack / ForceAttack   – UMAttackTask
 *   HoldPosition           – UMHoldPositionTask
 *   ForceMove              – UMMoveTask
 *   Retreat                – UMRetreatTask
 *   DestroyStorage         – UMAttackTargetTask
 *   Kidnap                 – UMKidnapTask
 *   AttackDefenseTower     – UMAttackTargetTask
 */
UCLASS()
class THELASTSHELTER_API AMOrdoAIController : public AMAIControllerBase
{
	GENERATED_BODY()

public:
	AMOrdoAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// ---- Command Pattern 인터페이스 ----
	virtual UMBaseTask* CreateTaskForType(EMTaskType TaskType, AActor* Target, const FVector& Location) override;
	virtual void ExecuteIdleBehavior(float DeltaTime) override;
	virtual void OnNewTaskStarted(UMBaseTask* Task) override;
	virtual AActor* ResolveAttackTarget() const override;

	UPROPERTY()
	TObjectPtr<AMOrdoCharacter> OwnerOrdo;

public:
	// ============================================================
	// 튜닝 파라미터
	// ============================================================

	/** 공격 사거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float AttackRange = 600.f;

	/** 순찰 범위 (스폰 지점 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float PatrolRadius = 500.f;

	/** 공격 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float AttackRate = 2.0f;

	/** 전투 시작 애니메이션 지속 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float CombatEnterDuration = 0.6f;

	/** 전투 진입 오프셋 — AttackRange에서 이 값만큼 더 충분히 접근해야 CombatEnter 진입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float CombatEngageOffset = 100.f;

	/** 퇴각 도착 허용 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float RetreatArrivalDist = 100.f;

private:
	FVector SpawnLocation;
	FVector PatrolTarget;
	bool Patrolling = false;

	/** 퇴각 목표 위치 (레벨 디자이너가 지정 또는 스폰 역방향) */
	FVector RetreatDestination;

	void ChooseNewPatrolPoint();
};
