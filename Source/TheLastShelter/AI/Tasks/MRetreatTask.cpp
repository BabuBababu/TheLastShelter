// Copyright TheLastShelter. All Rights Reserved.

#include "MRetreatTask.h"

UMRetreatTask::UMRetreatTask()
{
	TaskType = EMTaskType::Retreat;
	Category = EMTaskCategory::Combat;
}

void UMRetreatTask::StartTask()
{
	Super::StartTask();
	MoveToTargetLocation(TargetLocation, ArrivalDistance * 0.5f);
}

void UMRetreatTask::TickTask(float DeltaTime)
{
	const float dist = GetDistanceToLocation(TargetLocation);

	if (dist < ArrivalDistance)
	{
		StopOwnerMovement();

		if (bDestroyOnArrival && OwnerPawn)
		{
			UE_LOG(LogTemp, Log, TEXT("[OrdoAI] Retreat 도착 — 디스폰."));
			OwnerPawn->Destroy();
		}

		FinishTask(true);
		return;
	}

	if (!IsOwnerMoving())
	{
		MoveToTargetLocation(TargetLocation, ArrivalDistance * 0.5f);
	}
}
