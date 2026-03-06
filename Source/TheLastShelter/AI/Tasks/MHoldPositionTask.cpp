// Copyright TheLastShelter. All Rights Reserved.

#include "MHoldPositionTask.h"
#include "MAIControllerBase.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"

UMHoldPositionTask::UMHoldPositionTask()
{
	TaskType = EMTaskType::HoldPosition;
	Category = EMTaskCategory::Combat;
}

void UMHoldPositionTask::StartTask()
{
	Super::StartTask();
	LastAttackTime = -999.f;
	StopOwnerMovement();
}

void UMHoldPositionTask::TickTask(float DeltaTime)
{
	StopOwnerMovement();

	AActor* target = ResolveTarget();
	if (!target || IsTargetDead(target)) return;

	const float dist = GetDistanceTo(target);
	if (dist <= AttackRange)
	{
		const float now = GetWorldTime();
		if (now - LastAttackTime >= AttackRate)
		{
			if (AMEveCharacter* eve = GetOwnerEve())
			{
				eve->FaceTarget(target);
				eve->PerformAttack(target);
			}
			else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
			{
				ordo->FaceTarget(target);
				ordo->PerformAttack(target);
			}
			LastAttackTime = now;
		}
	}
}

AActor* UMHoldPositionTask::ResolveTarget() const
{
	if (TargetActor.IsValid() && !IsTargetDead(TargetActor.Get()))
		return TargetActor.Get();

	if (AMAIControllerBase* ctrl = GetBaseController())
		return ctrl->ResolveAttackTarget();

	return nullptr;
}

bool UMHoldPositionTask::IsTargetDead(AActor* Target) const
{
	if (!Target || Target->IsActorBeingDestroyed()) return true;
	if (const AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(Target)) return ordo->IsDead();
	if (const AMEveCharacter* eve = Cast<AMEveCharacter>(Target)) return eve->IsDead();
	return false;
}
