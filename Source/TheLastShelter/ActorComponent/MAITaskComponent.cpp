// Copyright TheLastShelter. All Rights Reserved.

#include "MAITaskComponent.h"

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

	// 시간 제한이 있는 태스크 자동 완료 체크
	if (CurrentTask.IsValid() && CurrentTask.TimeLimit > 0.f)
	{
		const float elapsed = GetWorld()->GetTimeSeconds() - CurrentTask.StartTime;
		if (elapsed >= CurrentTask.TimeLimit)
		{
			CompleteCurrentTask();
		}
	}

	// 현재 태스크 없으면 다음 태스크 시작
	if (!CurrentTask.IsValid() && TaskQueue.Num() > 0)
	{
		StartNextTask();
	}
}

void UMAITaskComponent::EnqueueTask(const FMAITask& Task)
{
	// Force/Interrupt 태스크는 InterruptWithTask 로 전환
	if (Task.IsForceTask())
	{
		InterruptWithTask(Task);
		return;
	}

	TaskQueue.Add(Task);

	UE_LOG(LogTemp, Log, TEXT("[AITask] Enqueued: %s (Queue: %d)"),
		*UEnum::GetValueAsString(Task.TaskType), TaskQueue.Num());

	// 현재 태스크가 없으면 즉시 시작
	if (!CurrentTask.IsValid())
	{
		StartNextTask();
	}
}

void UMAITaskComponent::InterruptWithTask(const FMAITask& Task)
{
	UE_LOG(LogTemp, Log, TEXT("[AITask] INTERRUPT: %s"), *UEnum::GetValueAsString(Task.TaskType));

	// 현재 태스크가 있으면 취소 후 큐 앞에 다시 넣기 (optional: 복구 가능)
	if (CurrentTask.IsValid())
	{
		CurrentTask.State = EMTaskState::Cancelled;
		// 취소된 태스크는 버림 — 필요 시 큐 앞에 다시 넣을 수 있음
	}

	// Force 태스크를 현재 태스크로 즉시 설정
	CurrentTask = Task;
	CurrentTask.State = EMTaskState::InProgress;
	CurrentTask.StartTime = GetWorld()->GetTimeSeconds();
	OnTaskStarted.Broadcast(CurrentTask);
}

void UMAITaskComponent::CompleteCurrentTask()
{
	if (!CurrentTask.IsValid()) return;

	CurrentTask.State = EMTaskState::Completed;

	UE_LOG(LogTemp, Log, TEXT("[AITask] Completed: %s"), *UEnum::GetValueAsString(CurrentTask.TaskType));
	OnTaskCompleted.Broadcast(CurrentTask);

	// 현재 태스크 초기화
	CurrentTask = FMAITask::MakeEmpty();

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

void UMAITaskComponent::CancelCurrentTask()
{
	if (!CurrentTask.IsValid()) return;

	CurrentTask.State = EMTaskState::Cancelled;
	UE_LOG(LogTemp, Log, TEXT("[AITask] Cancelled: %s"), *UEnum::GetValueAsString(CurrentTask.TaskType));

	CurrentTask = FMAITask::MakeEmpty();

	if (TaskQueue.Num() > 0)
	{
		StartNextTask();
	}
	else
	{
		OnTaskQueueEmpty.Broadcast();
	}
}

void UMAITaskComponent::ClearAllTasks()
{
	if (CurrentTask.IsValid())
	{
		CurrentTask.State = EMTaskState::Cancelled;
		CurrentTask = FMAITask::MakeEmpty();
	}
	TaskQueue.Empty();
	OnTaskQueueEmpty.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("[AITask] All tasks cleared."));
}

void UMAITaskComponent::RequestTask(EMTaskType TaskType, EMTaskCategory Category, AActor* Target,
	FVector Location, EMTaskPriority Priority)
{
	FMAITask task = FMAITask::Make(TaskType, Category, Priority);
	task.TargetActor = Target;
	task.TargetLocation = Location;

	EnqueueTask(task);
}

void UMAITaskComponent::StartNextTask()
{
	if (TaskQueue.Num() == 0) return;

	// 우선순위 가장 높은 태스크를 꺼냄
	int32 bestIndex = 0;
	EMTaskPriority bestPriority = TaskQueue[0].Priority;
	for (int32 i = 1; i < TaskQueue.Num(); ++i)
	{
		if (TaskQueue[i].Priority > bestPriority)
		{
			bestPriority = TaskQueue[i].Priority;
			bestIndex = i;
		}
	}

	CurrentTask = TaskQueue[bestIndex];
	TaskQueue.RemoveAt(bestIndex);

	CurrentTask.State = EMTaskState::InProgress;
	CurrentTask.StartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Log, TEXT("[AITask] Started: %s"), *UEnum::GetValueAsString(CurrentTask.TaskType));
	OnTaskStarted.Broadcast(CurrentTask);
}
