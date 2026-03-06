// Copyright TheLastShelter. All Rights Reserved.

#include "MKidnapTask.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "MAIControllerBase.h"
#include "MAITaskComponent.h"
#include "MRetreatTask.h"

UMKidnapTask::UMKidnapTask()
{
	TaskType = EMTaskType::Kidnap;
	Category = EMTaskCategory::Combat;
}

void UMKidnapTask::StartTask()
{
	Super::StartTask();
	bGrabbed = false;

	if (!TargetActor.IsValid())
	{
		FinishTask(true);
		return;
	}

	MoveToTarget(TargetActor.Get(), GrabDistance * 0.6f);
}

void UMKidnapTask::TickTask(float DeltaTime)
{
	AMEveCharacter* eve = Cast<AMEveCharacter>(TargetActor.Get());
	if (!eve)
	{
		FinishTask(true);
		return;
	}

	// Eve가 아직 기절하지 않았으면 → 공격은 별도 Attack 태스크에서 처리
	// 이 태스크는 기절 상태의 Eve만 대상
	if (!eve->IsDead())
	{
		// 기절하지 않은 Eve → 태스크 완료 (컨트롤러가 Attack 태스크 생성 결정)
		FinishTask(false);
		return;
	}

	const float dist = GetDistanceTo(eve);

	if (dist < GrabDistance && !bGrabbed)
	{
		// Eve 납치 → 부착
		bGrabbed = true;
		StopOwnerMovement();
		eve->AttachToActor(OwnerPawn.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		UE_LOG(LogTemp, Log, TEXT("[OrdoAI] 납치: %s"), *eve->GetName());

		// 퇴각 태스크 생성 → 큐에 추가
		if (AMAIControllerBase* ctrl = GetBaseController())
		{
			UMAITaskComponent* taskComp = OwnerPawn->FindComponentByClass<UMAITaskComponent>();
			if (taskComp)
			{
				UMRetreatTask* retreatTask = NewObject<UMRetreatTask>(OwnerController.Get());
				retreatTask->TaskType = EMTaskType::Retreat;
				retreatTask->TargetLocation = RetreatDestination;
				retreatTask->bDestroyOnArrival = true;
				retreatTask->Priority = EMTaskPriority::High;
				taskComp->EnqueueTask(retreatTask);
			}
		}

		FinishTask(true);
		return;
	}

	if (!bGrabbed && !IsOwnerMoving())
	{
		MoveToTarget(eve, GrabDistance * 0.6f);
	}
}
