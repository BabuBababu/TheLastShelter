// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MDataTypes.h"
#include "MAIControllerBase.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UMAITaskComponent;

/**
 * MAIControllerBase
 * Eve/Ordo AI 컨트롤러 공통 베이스.
 * AIPerception + AITaskComponent 기반 태스크 구동 패턴.
 *
 * 하위 클래스는 ExecuteTask()를 오버라이드하여 태스크별 행동을 구현.
 * Tick에서 자동으로 태스크 상태 머신을 구동함:
 *   Idle → (큐에서 꺼냄) → InProgress → (ExecuteTask) → Completed → 다음 체크
 */
UCLASS()
class THELASTSHELTER_API AMAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AMAIControllerBase();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// ============================================================
	// 태스크 시스템
	// ============================================================

	/** 소유 Pawn의 태스크 컴포넌트 (OnPossess에서 캐싱) */
	UPROPERTY(BlueprintReadOnly, Category = "AI|Task")
	TObjectPtr<UMAITaskComponent> TaskComp;

	/**
	 * 매 Tick 호출. 현재 태스크를 실행하는 핵심 가상 함수.
	 * 하위 클래스에서 오버라이드하여 태스크별 행동을 구현.
	 * 태스크가 끝났으면 TaskComp->CompleteCurrentTask() 를 호출.
	 */
	virtual void ExecuteTask(float DeltaTime, const FMAITask& Task);

	/**
	 * 현재 태스크가 없을 때(Idle) 매 Tick 호출.
	 * 하위 클래스에서 기본 행동(배회, 순찰 등)을 구현.
	 */
	virtual void ExecuteIdleBehavior(float DeltaTime);

	/**
	 * 태스크 시작 시 1회 호출 (OnTaskStarted 콜백).
	 * 상태 초기화 등에 사용.
	 */
	virtual void OnTaskBegin(const FMAITask& Task);

	// ============================================================
	// Perception
	// ============================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/** Perception 감지 콜백 */
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	/** 현재 감지된 적 */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> DetectedTarget;

	/** 나를 공격한 가장 최근의 적 (우선 타겟) */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TWeakObjectPtr<AActor> LastAttacker;

public:
	/** 시야 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float SightRadius = 1000.f;

	/** 시야 소실 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float LoseSightRadius = 1200.f;

	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetDetectedTarget() const { return DetectedTarget; }

	/** 나를 공격한 적 기록 (TakeDamage 로직에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetLastAttacker(AActor* Attacker) { LastAttacker = Attacker; }

	// ============================================================
	// 태스크 편의 API (Controller에서 바로 호출)
	// ============================================================

	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	void RequestTask(EMTaskType TaskType, EMTaskCategory Category, AActor* Target = nullptr,
		FVector Location = FVector::ZeroVector, EMTaskPriority Priority = EMTaskPriority::Normal);

	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	void ForceTask(EMTaskType TaskType, EMTaskCategory Category, AActor* Target = nullptr,
		FVector Location = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	void ClearAllTasks();

private:
	/** OnTaskStarted 콜백 래퍼 */
	UFUNCTION()
	void HandleTaskStarted(const FMAITask& Task);
};
