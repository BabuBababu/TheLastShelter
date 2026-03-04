// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MDataTypes.h"
#include "MAITaskComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskStarted, const FMAITask&, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompleted, const FMAITask&, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTaskQueueEmpty);

/**
 * MAITaskComponent
 * AI 태스크 큐 + 상태 머신을 제공하는 범용 컴포넌트.
 * AIController가 소유한 Pawn에 부착하거나,
 * AIController 자체에서 참조하여 사용.
 *
 * 태스크 흐름:
 *   Pending → InProgress → Completed → (다음 태스크 확인) → Execute or Idle
 *
 * Force 태스크(Interrupt)는 현재 태스크를 즉시 취소하고 삽입됨.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THELASTSHELTER_API UMAITaskComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAITaskComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================
	// 태스크 관리
	// ============================================================

	/** 태스크 큐 뒤에 추가 */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void EnqueueTask(const FMAITask& Task);

	/** Force/Interrupt 태스크: 현재 태스크 취소 후 큐 맨 앞에 삽입 */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void InterruptWithTask(const FMAITask& Task);

	/** 현재 태스크 완료 처리 → 다음 태스크 진행 */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void CompleteCurrentTask();

	/** 현재 태스크 취소 → 다음 태스크 진행 */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void CancelCurrentTask();

	/** 큐 전체 비우기 (현재 태스크 포함) */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void ClearAllTasks();

	/** 큐에 대기 중인 태스크 수 (현재 태스크 제외) */
	UFUNCTION(BlueprintPure, Category = "AITask")
	int32 GetQueuedTaskCount() const { return TaskQueue.Num(); }

	/** 현재 실행 중인 태스크가 있는지 */
	UFUNCTION(BlueprintPure, Category = "AITask")
	bool HasCurrentTask() const { return CurrentTask.IsValid(); }

	/** 현재 태스크 반환 */
	UFUNCTION(BlueprintPure, Category = "AITask")
	const FMAITask& GetCurrentTask() const { return CurrentTask; }

	/** 큐가 비어있고 현재 태스크도 없으면 true */
	UFUNCTION(BlueprintPure, Category = "AITask")
	bool IsIdle() const { return !CurrentTask.IsValid() && TaskQueue.Num() == 0; }

	// ============================================================
	// 태스크 편의 함수
	// ============================================================

	/** 태스크 간편 예약 */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void RequestTask(EMTaskType TaskType, EMTaskCategory Category, AActor* Target = nullptr,
		FVector Location = FVector::ZeroVector, EMTaskPriority Priority = EMTaskPriority::Normal);

	// ============================================================
	// 델리게이트
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "AITask")
	FOnTaskStarted OnTaskStarted;

	UPROPERTY(BlueprintAssignable, Category = "AITask")
	FOnTaskCompleted OnTaskCompleted;

	UPROPERTY(BlueprintAssignable, Category = "AITask")
	FOnTaskQueueEmpty OnTaskQueueEmpty;

private:
	/** 현재 실행 중인 태스크 */
	UPROPERTY()
	FMAITask CurrentTask;

	/** 대기 중인 태스크 큐 */
	UPROPERTY()
	TArray<FMAITask> TaskQueue;

	/** 다음 태스크 꺼내서 시작 */
	void StartNextTask();
};
