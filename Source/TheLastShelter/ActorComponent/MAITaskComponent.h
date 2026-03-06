// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MDataTypes.h"
#include "MBaseTask.h"
#include "MAITaskComponent.generated.h"

class AAIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewTaskStarted, UMBaseTask*, Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTaskQueueEmpty);

/**
 * MAITaskComponent
 * AI 태스크 큐 + 라이프사이클 관리 컴포넌트 (Command Pattern).
 *
 * UMBaseTask* 태스크 객체를 큐로 관리하며,
 * TickComponent에서 현재 태스크의 TickTask()를 매 틱 구동.
 *
 * 태스크 흐름:
 *   [큐 삽입] → Initialize → StartTask → TickTask(매 틱) → EndTask → [다음 태스크]
 *
 * Force/Interrupt 태스크는 현재 태스크를 즉시 취소하고 교체.
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

	/** 태스크 객체를 큐에 추가. Force 태스크는 자동으로 InterruptWithTask 처리. */
	void EnqueueTask(UMBaseTask* Task);

	/** Force/Interrupt 태스크: 현재 태스크 취소 후 즉시 교체 */
	void InterruptWithTask(UMBaseTask* Task);

	/** 큐 전체 비우기 (현재 태스크 포함) */
	UFUNCTION(BlueprintCallable, Category = "AITask")
	void ClearAllTasks();

	/** 큐에 대기 중인 태스크 수 (현재 태스크 제외) */
	UFUNCTION(BlueprintPure, Category = "AITask")
	int32 GetQueuedTaskCount() const { return TaskQueue.Num(); }

	/** 현재 실행 중인 태스크가 있는지 */
	UFUNCTION(BlueprintPure, Category = "AITask")
	bool HasCurrentTask() const { return CurrentTask != nullptr && CurrentTask->IsActive(); }

	/** 현재 태스크 반환 (없으면 nullptr) */
	UMBaseTask* GetCurrentTask() const { return CurrentTask; }

	/** 큐가 비어있고 현재 태스크도 없으면 true */
	UFUNCTION(BlueprintPure, Category = "AITask")
	bool IsIdle() const { return CurrentTask == nullptr && TaskQueue.Num() == 0; }

	// ============================================================
	// 델리게이트
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "AITask")
	FOnNewTaskStarted OnTaskStarted;

	UPROPERTY(BlueprintAssignable, Category = "AITask")
	FOnTaskQueueEmpty OnTaskQueueEmpty;

	// ============================================================
	// 오너 컨트롤러 캐싱
	// ============================================================

	/** 소유 AIController (OnPossess 후 컨트롤러가 설정) */
	void SetOwnerController(AAIController* InController) { CachedController = InController; }
	AAIController* GetOwnerController() const { return CachedController; }

private:
	/** 현재 실행 중인 태스크 */
	UPROPERTY()
	TObjectPtr<UMBaseTask> CurrentTask;

	/** 대기 중인 태스크 큐 */
	UPROPERTY()
	TArray<TObjectPtr<UMBaseTask>> TaskQueue;

	/** 캐싱된 오너 컨트롤러 */
	UPROPERTY()
	TObjectPtr<AAIController> CachedController;

	/** 다음 태스크 꺼내서 시작 */
	void StartNextTask();

	/** 현재 태스크 완료/취소 콜백 */
	void OnCurrentTaskFinished(UMBaseTask* Task, bool bSuccess);
};
