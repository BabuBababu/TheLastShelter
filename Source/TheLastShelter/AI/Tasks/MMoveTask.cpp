// Copyright TheLastShelter. All Rights Reserved.

#include "MMoveTask.h"

UMMoveTask::UMMoveTask()
{
	TaskType = EMTaskType::Move;
	Category = EMTaskCategory::Life;
}

void UMMoveTask::StartTask()
{
	Super::StartTask();

	// 즉시 이동 시작
	if (TargetActor.IsValid())
	{
		MoveToTarget(TargetActor.Get(), ArrivalDistance * 0.6f);
	}
	else
	{
		MoveToTargetLocation(TargetLocation, ArrivalDistance * 0.6f);
	}
}

void UMMoveTask::TickTask(float DeltaTime)
{
	const FVector destination = GetDestination();
	const float dist = GetDistanceToLocation(destination);

	if (dist < ArrivalDistance)
	{
		StopOwnerMovement();
		FinishTask(true);
		return;
	}

	// 이동 중이 아니면 재요청
	if (!IsOwnerMoving())
	{
		if (TargetActor.IsValid())
		{
			MoveToTarget(TargetActor.Get(), ArrivalDistance * 0.6f);
		}
		else
		{
			MoveToTargetLocation(TargetLocation, ArrivalDistance * 0.6f);
		}
	}
}

FVector UMMoveTask::GetDestination() const
{
	if (TargetActor.IsValid())
		return TargetActor->GetActorLocation();
	return TargetLocation;
}
