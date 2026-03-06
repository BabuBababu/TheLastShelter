// Copyright TheLastShelter. All Rights Reserved.

#include "MCombatExitTask.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Engine/Engine.h"

UMCombatExitTask::UMCombatExitTask()
{
	TaskType = EMTaskType::None;
	Category = EMTaskCategory::Combat;
}

void UMCombatExitTask::StartTask()
{
	Super::StartTask();
	PhaseStartTime = GetWorldTime();

	StopOwnerMovement();
	ApplyCombatExitAnim();

	const FString ownerName = GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo");
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange,
		FString::Printf(TEXT("[%s] >> CombatExitTask 시작 (역재생 %.1fs)"), *ownerName, Duration));
}

void UMCombatExitTask::TickTask(float DeltaTime)
{
	const float elapsed = GetWorldTime() - PhaseStartTime;

	if (elapsed >= Duration)
	{
		RestoreToIdle();

		const FString ownerName = GetOwnerEve() ? TEXT("Eve") : TEXT("Ordo");
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White,
			FString::Printf(TEXT("[%s] CombatExitTask 완료 → GunIdle"), *ownerName));

		FinishTask(true);
	}
}

void UMCombatExitTask::EndTask(bool bWasCancelled)
{
	if (bWasCancelled)
	{
		// 강제 취소 시에도 GunIdle로 복원
		RestoreToIdle();
	}
	Super::EndTask(bWasCancelled);
}

void UMCombatExitTask::ApplyCombatExitAnim()
{
	if (AMEveCharacter* eve = GetOwnerEve())
	{
		eve->PlayCombatExitAnim();
	}
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
	{
		ordo->PlayCombatExitAnim();
	}
}

void UMCombatExitTask::RestoreToIdle()
{
	if (AMEveCharacter* eve = GetOwnerEve())
	{
		if (eve->GetSprite()) eve->GetSprite()->SetPlayRate(1.0f);
		eve->SetAnimState(EMEveAnimState::GunIdle);
	}
	else if (AMOrdoCharacter* ordo = GetOwnerOrdo())
	{
		if (ordo->GetSprite()) ordo->GetSprite()->SetPlayRate(1.0f);
		ordo->SetAnimState(EMOrdoAnimState::GunIdle);
	}
}
