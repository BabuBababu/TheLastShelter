// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MDataTypes.h"
#include "MAIControllerBase.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UMAITaskComponent;
class UMBaseTask;

/**
 * MAIControllerBase
 * Eve/Ordo AI 컨트롤러 공통 베이스.
 * AIPerception + Command Pattern TaskComponent 기반.
 *
 * 하위 클래스는 CreateTaskForType()를 오버라이드하여 태스크 생성 팩토리를 구현.
 * 태스크 실행은 UMBaseTask::TickTask()가 스스로 처리 (컨트롤러에서 직접 구동하지 않음).
 * 컨트롤러의 Tick에서는 Idle 행동만 구동.
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
	 * 태스크 팩토리 — EMTaskType에 맞는 UMBaseTask* 생성.
	 * 하위 클래스에서 오버라이드하여 적절한 태스크 객체 생성 + 설정.
	 * @return 생성된 태스크 (nullptr이면 지원하지 않는 태스크)
	 */
	virtual UMBaseTask* CreateTaskForType(EMTaskType TaskType, AActor* Target, const FVector& Location);

	/**
	 * 현재 태스크가 없을 때(Idle) 매 Tick 호출.
	 * 하위 클래스에서 기본 행동(배회, 순찰, 적 감지 시 전투 태스크 생성 등)을 구현.
	 */
	virtual void ExecuteIdleBehavior(float DeltaTime);

	/**
	 * 새 태스크 시작 시 호출 (OnTaskStarted 콜백).
	 * 하위 클래스에서 컨트롤러 레벨 초기화.
	 */
	virtual void OnNewTaskStarted(UMBaseTask* Task);

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

	/**
	 * 우선 타겟 결정 — 하위 클래스에서 오버라이드.
	 * LastAttacker가 사거리 내이면 우선, 아니면 DetectedTarget.
	 * 태스크 객체가 타겟을 찾을 때 이 함수를 사용.
	 */
	virtual AActor* ResolveAttackTarget() const;

	// ============================================================
	// 태스크 편의 API (외부 시스템에서 호출)
	// ============================================================

	/** 태스크 예약 — CreateTaskForType()으로 생성 후 큐에 추가 */
	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	void RequestTask(EMTaskType TaskType, EMTaskCategory Category, AActor* Target = nullptr,
		FVector Location = FVector::ZeroVector, EMTaskPriority Priority = EMTaskPriority::Normal);

	/** Force 태스크 — Interrupt 우선순위로 즉시 교체 */
	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	void ForceTask(EMTaskType TaskType, EMTaskCategory Category, AActor* Target = nullptr,
		FVector Location = FVector::ZeroVector);

	/** 모든 태스크 클리어 */
	UFUNCTION(BlueprintCallable, Category = "AI|Task")
	void ClearAllTasks();

private:
	/** OnTaskStarted 콜백 래퍼 */
	UFUNCTION()
	void HandleTaskStarted(UMBaseTask* Task);
};
