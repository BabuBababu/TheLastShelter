// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MAIControllerBase.h"
#include "MEveAIController.generated.h"

class AMEveCharacter;

/**
 * MEveAIController
 * Eve(미소녀) 전용 AI 컨트롤러. 플레이어 편.
 *
 * === 전투 태스크 ===
 *   Attack          – 감지된 적 공격 (나를 공격한 대상 우선)
 *   ForceAttack     – 사거리 밖이면 이동 후 강제 공격
 *   HoldPosition    – 현재 위치 고수 (이동 금지)
 *   ForceMove       – 지정 위치로 강제 이동
 *   RetreatToShelter– 숙소로 퇴각
 *   UseSkill        – 스킬 슬롯(0~2) 사용
 *
 * === 생활 태스크 ===
 *   Idle            – 대기 (멘탈 성향 기반 랜덤 행동)
 *   Move / ForceMove– 이동 / 강제 이동
 *   Carry / ForceCarry – 운반
 *   Rest / ForceRest   – 휴식
 *   Sex / ForceSex     – 섹스
 *   Masturbation / ForceMasturbation – 자위
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

	// ---- 태스크 오버라이드 ----
	virtual void ExecuteTask(float DeltaTime, const FMAITask& Task) override;
	virtual void ExecuteIdleBehavior(float DeltaTime) override;
	virtual void OnTaskBegin(const FMAITask& Task) override;

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
	float AttackRange = 150.f;

	/** 공격 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float AttackRate = 1.5f;

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

private:
	float LastAttackTime = -999.f;

	// ---- 배회 ----
	FVector WanderTarget = FVector::ZeroVector;
	bool IsWandering = false;
	float WanderIdleEndTime = 0.f;
	bool IsWanderIdle = false;
	void ChooseNewWanderPoint();

	// ---- 태스크별 내부 타이머 ----
	float TaskActionStartTime = 0.f;

	// ---- 전투 태스크 실행 ----
	void ExecuteAttack(float DeltaTime, const FMAITask& Task);
	void ExecuteForceAttack(float DeltaTime, const FMAITask& Task);
	void ExecuteHoldPosition(float DeltaTime);
	void ExecuteForceMove(float DeltaTime, const FMAITask& Task);
	void ExecuteRetreatToShelter(float DeltaTime);
	void ExecuteUseSkill(float DeltaTime, const FMAITask& Task);

	// ---- 생활 태스크 실행 ----
	void ExecuteIdle(float DeltaTime);
	void ExecuteMove(float DeltaTime, const FMAITask& Task);
	void ExecuteCarry(float DeltaTime, const FMAITask& Task);
	void ExecuteRest(float DeltaTime);
	void ExecuteSex(float DeltaTime, const FMAITask& Task);
	void ExecuteMasturbation(float DeltaTime);

	/** 우선 타겟 결정 — LastAttacker가 사거리 내이면 우선, 아니면 DetectedTarget */
	AActor* ResolveAttackTarget() const;
};
