// Copyright TheLastShelter. All Rights Reserved.

#include "MIdleTask.h"
#include "NavigationSystem.h"

UMIdleTask::UMIdleTask()
{
	TaskType = EMTaskType::Idle;
	Category = EMTaskCategory::Life;
}

void UMIdleTask::StartTask()
{
	Super::StartTask();
	BeginWaiting();
}

void UMIdleTask::TickTask(float DeltaTime)
{
	// 전체 지속 시간 체크
	if (TotalDuration > 0.f && GetTaskElapsedTime() >= TotalDuration)
	{
		StopOwnerMovement();
		FinishTask(true);
		return;
	}

	switch (CurrentPhase)
	{
	case EIdlePhase::Waiting:
	{
		if (GetWorldTime() >= WaitEndTime)
		{
			ChooseNewWanderPoint();
			CurrentPhase = EIdlePhase::Moving;
			MoveToTargetLocation(WanderTarget, 30.f);
		}
		break;
	}

	case EIdlePhase::Moving:
	{
		const float dist = GetDistanceToLocation(WanderTarget);
		if (dist < 50.f || !IsOwnerMoving())
		{
			StopOwnerMovement();
			BeginWaiting();
		}
		break;
	}
	}
}

void UMIdleTask::ChooseNewWanderPoint()
{
	UWorld* world = GetTaskWorld();
	if (!world) return;

	UNavigationSystemV1* navSys = UNavigationSystemV1::GetCurrent(world);
	if (navSys)
	{
		FNavLocation result;
		if (navSys->GetRandomReachablePointInRadius(WanderOrigin, WanderRadius, result))
		{
			WanderTarget = result.Location;
			return;
		}
	}

	// NavMesh 실패 시 랜덤 오프셋
	WanderTarget = WanderOrigin + FVector(
		FMath::RandRange(-WanderRadius, WanderRadius),
		FMath::RandRange(-WanderRadius, WanderRadius),
		0.f);
}

void UMIdleTask::BeginWaiting()
{
	CurrentPhase = EIdlePhase::Waiting;
	WaitEndTime = GetWorldTime() + FMath::RandRange(WanderIdleTime * 0.5f, WanderIdleTime * 1.5f);
}
