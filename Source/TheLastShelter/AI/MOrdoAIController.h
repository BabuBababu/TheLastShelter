// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MAIControllerBase.h"
#include "MOrdoAIController.generated.h"

class AMOrdoCharacter;
class AMEveCharacter;

/**
 * MOrdoAIController
 * Ordo(적) 전용 AI 컨트롤러.
 *
 * === 전투 태스크 ===
 *   Attack             – 플레이어/Eve 공격 (나를 공격한 대상 우선)
 *   ForceAttack        – 강제 공격 (사거리 무시 추격)
 *   ForceMove          – 지정 위치로 강제 이동
 *   HoldPosition       – 위치 고수
 *   Retreat            – 레벨 지정 스팟으로 퇴각 후 디스폰
 *   DestroyStorage     – 창고(Storage) 부수기
 *   Kidnap             – 기절한 Eve를 엎고 퇴각
 *   AttackDefenseTower – 데미지 무시, 타워만 집중 공격
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

	// ---- 태스크 오버라이드 ----
	virtual void ExecuteTask(float DeltaTime, const FMAITask& Task) override;
	virtual void ExecuteIdleBehavior(float DeltaTime) override;
	virtual void OnTaskBegin(const FMAITask& Task) override;

	UPROPERTY()
	TObjectPtr<AMOrdoCharacter> OwnerOrdo;

public:
	// ============================================================
	// 튜닝 파라미터
	// ============================================================

	/** 공격 사거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float AttackRange = 120.f;

	/** 순찰 범위 (스폰 지점 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float PatrolRadius = 500.f;

	/** 공격 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float AttackRate = 2.0f;

	/** 퇴각 도착 허용 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float RetreatArrivalDist = 100.f;

private:
	FVector SpawnLocation;
	FVector PatrolTarget;
	bool Patrolling = false;
	float LastAttackTime = -999.f;

	/** 퇴각 목표 위치 (레벨 디자이너가 지정 또는 스폰 역방향) */
	FVector RetreatDestination;

	/** 납치 중인 Eve */
	UPROPERTY()
	TWeakObjectPtr<AMEveCharacter> KidnapTarget;

	void ChooseNewPatrolPoint();

	// ---- 전투 태스크 실행 ----
	void ExecuteAttack(float DeltaTime, const FMAITask& Task);
	void ExecuteForceAttack(float DeltaTime, const FMAITask& Task);
	void ExecuteHoldPosition(float DeltaTime);
	void ExecuteForceMove(float DeltaTime, const FMAITask& Task);
	void ExecuteRetreat(float DeltaTime, const FMAITask& Task);
	void ExecuteDestroyStorage(float DeltaTime, const FMAITask& Task);
	void ExecuteKidnap(float DeltaTime, const FMAITask& Task);
	void ExecuteAttackDefenseTower(float DeltaTime, const FMAITask& Task);

	/** 우선 타겟 결정 — LastAttacker 우선, 아니면 감지된 적 */
	AActor* ResolveAttackTarget() const;
};
