// Copyright TheLastShelter. All Rights Reserved.

#include "MAITaskComponent.h"
#include "MBaseTask.h"

UMAITaskComponent::UMAITaskComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMAITaskComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMAITaskComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 현재 태스크가 있으면 TickTask 구동
	if (CurrentTask && CurrentTask->IsActive())
	{
		CurrentTask->TickTask(DeltaTime);
	}

	// 현재 태스크가 끝났거나 없으면 다음 태스크 시작
	if ((!CurrentTask || CurrentTask->IsFinished()) && TaskQueue.Num() > 0)
	{
		CurrentTask = nullptr;
		StartNextTask();
	}
}

// ============================================================
// 태스크 관리
// ============================================================

void UMAITaskComponent::EnqueueTask(UMBaseTask* Task)
{
	if (!Task) return;

	// Force/Interrupt 태스크는 InterruptWithTask로 전환
	if (Task->IsForceTask())
	{
		InterruptWithTask(Task);
		return;
	}

	TaskQueue.Add(Task);

	UE_LOG(LogTemp, Log, TEXT("[AITask] Enqueued: %s (Queue: %d)"),
		*UEnum::GetValueAsString(Task->TaskType), TaskQueue.Num());

	// 현재 태스크가 없으면 즉시 시작
	if (!CurrentTask || !CurrentTask->IsActive())
	{
		CurrentTask = nullptr;
		StartNextTask();
	}
}

void UMAITaskComponent::InterruptWithTask(UMBaseTask* Task)
{
	if (!Task) return;

	UE_LOG(LogTemp, Log, TEXT("[AITask] INTERRUPT: %s"), *UEnum::GetValueAsString(Task->TaskType));

	// 현재 태스크 취소
	if (CurrentTask && CurrentTask->IsActive())
	{
		CurrentTask->OnTaskFinished.Unbind();
		CurrentTask->CancelTask();
	}

	// 새 태스크를 현재 태스크로 설정
	CurrentTask = Task;

	// Initialize + Start
	if (CachedController)
	{
		CurrentTask->Initialize(CachedController);
	}
	CurrentTask->OnTaskFinished.BindUObject(this, &UMAITaskComponent::OnCurrentTaskFinished);
	CurrentTask->StartTask();

	OnTaskStarted.Broadcast(CurrentTask);
}

void UMAITaskComponent::ClearAllTasks()
{
	// 현재 태스크 취소
	if (CurrentTask && CurrentTask->IsActive())
	{
		CurrentTask->OnTaskFinished.Unbind();
		CurrentTask->CancelTask();
	}
	CurrentTask = nullptr;

	// 큐 비우기
	TaskQueue.Empty();
	OnTaskQueueEmpty.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[AITask] All tasks cleared."));
}

// ============================================================
// 내부
// ============================================================

void UMAITaskComponent::StartNextTask()
{
	if (TaskQueue.Num() == 0)
	{
		OnTaskQueueEmpty.Broadcast();
		return;
	}

	// 우선순위 가장 높은 태스크를 꺼냄
	int32 bestIndex = 0;
	EMTaskPriority bestPriority = TaskQueue[0]->Priority;
	for (int32 i = 1; i < TaskQueue.Num(); ++i)
	{
		if (TaskQueue[i]->Priority > bestPriority)
		{
			bestPriority = TaskQueue[i]->Priority;
			bestIndex = i;
		}
	}

	CurrentTask = TaskQueue[bestIndex];
	TaskQueue.RemoveAt(bestIndex);

	// Initialize + Start
	if (CachedController)
	{
		CurrentTask->Initialize(CachedController);
	}
	CurrentTask->OnTaskFinished.BindUObject(this, &UMAITaskComponent::OnCurrentTaskFinished);
	CurrentTask->StartTask();

	UE_LOG(LogTemp, Log, TEXT("[AITask] Started: %s"), *UEnum::GetValueAsString(CurrentTask->TaskType));
	OnTaskStarted.Broadcast(CurrentTask);
}

void UMAITaskComponent::OnCurrentTaskFinished(UMBaseTask* Task, bool bSuccess)
{
	UE_LOG(LogTemp, Log, TEXT("[AITask] %s: %s"),
		bSuccess ? TEXT("Completed") : TEXT("Failed"),
		*UEnum::GetValueAsString(Task->TaskType));

	// 현재 태스크 정리
	if (CurrentTask == Task)
	{
		CurrentTask->OnTaskFinished.Unbind();
		CurrentTask = nullptr;
	}

	// 다음 태스크 시작
	if (TaskQueue.Num() > 0)
	{
		StartNextTask();
	}
	else
	{
		OnTaskQueueEmpty.Broadcast();
	}
}
