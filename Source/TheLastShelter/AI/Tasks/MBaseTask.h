// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MDataTypes.h"
#include "MBaseTask.generated.h"

class AAIController;
class AMEveCharacter;
class AMOrdoCharacter;
class AMAIControllerBase;

/**
 * UMBaseTask — AI 태스크 기본 클래스 (Command Pattern).
 *
 * 모든 AI 행동(전투, 생활 등)은 이 클래스를 상속하여 구현.
 * 각 태스크 객체가 자체 상태 머신을 보유:
 *   Initialize → StartTask → TickTask(매 틱) → EndTask
 *
 * TaskComponent가 태스크의 라이프사이클을 구동하며,
 * 태스크는 완료 시 FinishTask()를 호출하여 알림.
 */
UCLASS(Abstract, Blueprintable)
class THELASTSHELTER_API UMBaseTask : public UObject
{
	GENERATED_BODY()

public:
	// ============================================================
	// 태스크 설정 (EnqueueTask 전에 팩토리에서 설정)
	// ============================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	EMTaskType TaskType = EMTaskType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	EMTaskCategory Category = EMTaskCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	EMTaskPriority Priority = EMTaskPriority::Normal;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(BlueprintReadWrite, Category = "Task")
	FVector TargetLocation = FVector::ZeroVector;

	/** 스킬 슬롯 인덱스 (UseSkill 전용) */
	UPROPERTY(BlueprintReadWrite, Category = "Task")
	int32 SkillIndex = -1;

	// ============================================================
	// 라이프사이클
	// ============================================================

	/** TaskComponent가 태스크 시작 직전에 호출. 오너 참조 설정. */
	void Initialize(AAIController* InController);

	/** 태스크 실행 시작 (1회 호출) */
	virtual void StartTask();

	/** 매 틱 호출 (활성 상태일 때) */
	virtual void TickTask(float DeltaTime);

	/** 태스크 종료 시 호출 (정리). bWasCancelled=true이면 중간 취소. */
	virtual void EndTask(bool bWasCancelled);

	/** 태스크 스스로 완료 신호 — TaskComponent에 알림 */
	void FinishTask(bool bSuccess);

	/** 외부에서 태스크 취소 */
	void CancelTask();

	// ============================================================
	// 상태 조회
	// ============================================================

	bool IsActive() const { return bIsActive; }
	bool IsFinished() const { return bIsFinished; }
	bool IsForceTask() const;

	// ============================================================
	// 완료 딜리게이트 (TaskComponent가 바인딩)
	// ============================================================

	DECLARE_DELEGATE_TwoParams(FOnMBaseTaskFinished, UMBaseTask* /*Task*/, bool /*bSuccess*/);
	FOnMBaseTaskFinished OnTaskFinished;

protected:
	// ============================================================
	// 오너 참조
	// ============================================================

	UPROPERTY()
	TObjectPtr<AAIController> OwnerController;

	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	// ============================================================
	// 유틸리티
	// ============================================================

	UWorld* GetTaskWorld() const;
	float GetWorldTime() const;
	float GetTaskElapsedTime() const;

	/** 오너 Pawn을 Eve로 캐스트 (Eve가 아니면 nullptr) */
	AMEveCharacter* GetOwnerEve() const;

	/** 오너 Pawn을 Ordo로 캐스트 (Ordo가 아니면 nullptr) */
	AMOrdoCharacter* GetOwnerOrdo() const;

	/** 오너 컨트롤러를 MAIControllerBase로 캐스트 */
	AMAIControllerBase* GetBaseController() const;

	// ---- 내비게이션 헬퍼 ----

	/** MoveToActor 래퍼 */
	void MoveToTarget(AActor* Target, float AcceptanceRadius);

	/** MoveToLocation 래퍼 */
	void MoveToTargetLocation(const FVector& Location, float AcceptanceRadius);

	/** StopMovement 래퍼 */
	void StopOwnerMovement();

	/** PathFollowing이 Moving 상태인지 */
	bool IsOwnerMoving() const;

	/** 오너 Pawn과 타겟 사이의 거리 */
	float GetDistanceTo(AActor* Target) const;

	/** 오너 Pawn과 위치 사이의 거리 */
	float GetDistanceToLocation(const FVector& Location) const;

private:
	bool bIsActive = false;
	bool bIsFinished = false;
	float StartTime = 0.f;
};
