// Copyright TheLastShelter. All Rights Reserved.

#include "MCarryTask.h"
#include "MEveCharacter.h"

UMCarryTask::UMCarryTask()
{
	TaskType = EMTaskType::Carry;
	Category = EMTaskCategory::Life;
}

void UMCarryTask::StartTask()
{
	Super::StartTask();

	if (!TargetActor.IsValid())
	{
		FinishTask(true);
		return;
	}

	MoveToTarget(TargetActor.Get(), PickupDistance * 0.6f);
}

void UMCarryTask::TickTask(float DeltaTime)
{
	if (!TargetActor.IsValid())
	{
		FinishTask(true);
		return;
	}

	const float dist = GetDistanceTo(TargetActor.Get());
	if (dist < PickupDistance)
	{
		StopOwnerMovement();

		if (AMEveCharacter* eve = GetOwnerEve())
		{
			eve->SetAnimState(EMEveAnimState::BoxCarry_Down);
		}

		UE_LOG(LogTemp, Log, TEXT("[EveAI] Carry: 대상 도착, 운반 완료."));
		FinishTask(true);
		return;
	}

	if (!IsOwnerMoving())
	{
		MoveToTarget(TargetActor.Get(), PickupDistance * 0.6f);
	}
}
