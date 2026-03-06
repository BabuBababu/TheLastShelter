// Copyright TheLastShelter. All Rights Reserved.

#include "MAttackTargetTask.h"
#include "MOrdoCharacter.h"

UMAttackTargetTask::UMAttackTargetTask()
{
	Category = EMTaskCategory::Combat;
}

void UMAttackTargetTask::StartTask()
{
	Super::StartTask();
	LastAttackTime = -999.f;

	if (!TargetActor.IsValid())
	{
		FinishTask(true);
		return;
	}

	MoveToTarget(TargetActor.Get(), AttackRange * 0.5f);
}

void UMAttackTargetTask::TickTask(float DeltaTime)
{
	if (!TargetActor.IsValid() || TargetActor->IsActorBeingDestroyed())
	{
		FinishTask(true);
		return;
	}

	const float dist = GetDistanceTo(TargetActor.Get());

	if (dist <= AttackRange)
	{
		StopOwnerMovement();

		const float now = GetWorldTime();
		if (now - LastAttackTime >= AttackRate)
		{
			if (AMOrdoCharacter* ordo = GetOwnerOrdo())
			{
				ordo->FaceTarget(TargetActor.Get());
				ordo->PerformAttack(TargetActor.Get());
			}
			LastAttackTime = now;
		}
	}
	else
	{
		if (!IsOwnerMoving())
		{
			MoveToTarget(TargetActor.Get(), AttackRange * 0.5f);
		}
	}
}
