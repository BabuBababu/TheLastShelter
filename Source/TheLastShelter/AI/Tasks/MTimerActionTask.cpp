// Copyright TheLastShelter. All Rights Reserved.

#include "MTimerActionTask.h"
#include "MEveCharacter.h"

UMTimerActionTask::UMTimerActionTask()
{
	Category = EMTaskCategory::Life;
}

void UMTimerActionTask::StartTask()
{
	Super::StartTask();
	StopOwnerMovement();

	// Eve 애니메이션 설정
	if (AMEveCharacter* eve = GetOwnerEve())
	{
		eve->SetAnimState(EveAnimState);
	}
}

void UMTimerActionTask::TickTask(float DeltaTime)
{
	StopOwnerMovement();

	if (GetTaskElapsedTime() >= Duration)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] TimerAction (%s) 완료."),
			GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo"),
			*UEnum::GetValueAsString(TaskType));
		FinishTask(true);
	}
}
