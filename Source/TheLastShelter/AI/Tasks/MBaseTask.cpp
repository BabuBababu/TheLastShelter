// Copyright TheLastShelter. All Rights Reserved.

#include "MBaseTask.h"
#include "AIController.h"
#include "MEveCharacter.h"
#include "MOrdoCharacter.h"
#include "MAIControllerBase.h"
#include "Navigation/PathFollowingComponent.h"

// ============================================================
// 라이프사이클
// ============================================================

void UMBaseTask::Initialize(AAIController* InController)
{
	OwnerController = InController;
	OwnerPawn = InController ? InController->GetPawn() : nullptr;
}

void UMBaseTask::StartTask()
{
	bIsActive = true;
	bIsFinished = false;
	StartTime = GetWorldTime();
}

void UMBaseTask::TickTask(float DeltaTime)
{
	// 서브클래스에서 오버라이드
}

void UMBaseTask::EndTask(bool bWasCancelled)
{
	bIsActive = false;
}

void UMBaseTask::FinishTask(bool bSuccess)
{
	if (bIsFinished) return;
	bIsFinished = true;
	bIsActive = false;
	EndTask(false);
	OnTaskFinished.ExecuteIfBound(this, bSuccess);
}

void UMBaseTask::CancelTask()
{
	if (bIsFinished) return;
	EndTask(true);
	bIsFinished = true;
	bIsActive = false;
	OnTaskFinished.ExecuteIfBound(this, false);
}

// ============================================================
// 상태 조회
// ============================================================

bool UMBaseTask::IsForceTask() const
{
	return Priority == EMTaskPriority::Interrupt
		|| TaskType == EMTaskType::ForceAttack
		|| TaskType == EMTaskType::ForceMove
		|| TaskType == EMTaskType::ForceCarry
		|| TaskType == EMTaskType::ForceRest
		|| TaskType == EMTaskType::ForceSex
		|| TaskType == EMTaskType::ForceMasturbation;
}

// ============================================================
// 유틸리티
// ============================================================

UWorld* UMBaseTask::GetTaskWorld() const
{
	return OwnerPawn ? OwnerPawn->GetWorld() : nullptr;
}

float UMBaseTask::GetWorldTime() const
{
	UWorld* world = GetTaskWorld();
	return world ? world->GetTimeSeconds() : 0.f;
}

float UMBaseTask::GetTaskElapsedTime() const
{
	return GetWorldTime() - StartTime;
}

AMEveCharacter* UMBaseTask::GetOwnerEve() const
{
	return Cast<AMEveCharacter>(OwnerPawn.Get());
}

AMOrdoCharacter* UMBaseTask::GetOwnerOrdo() const
{
	return Cast<AMOrdoCharacter>(OwnerPawn.Get());
}

AMAIControllerBase* UMBaseTask::GetBaseController() const
{
	return Cast<AMAIControllerBase>(OwnerController.Get());
}

// ============================================================
// 내비게이션 헬퍼
// ============================================================

void UMBaseTask::MoveToTarget(AActor* Target, float AcceptanceRadius)
{
	if (OwnerController && Target)
	{
		OwnerController->MoveToActor(Target, AcceptanceRadius);
	}
}

void UMBaseTask::MoveToTargetLocation(const FVector& Location, float AcceptanceRadius)
{
	if (OwnerController)
	{
		OwnerController->MoveToLocation(Location, AcceptanceRadius);
	}
}

void UMBaseTask::StopOwnerMovement()
{
	if (OwnerController)
	{
		OwnerController->StopMovement();
	}
}

bool UMBaseTask::IsOwnerMoving() const
{
	if (!OwnerController) return false;
	UPathFollowingComponent* pathComp = OwnerController->GetPathFollowingComponent();
	return pathComp && pathComp->GetStatus() == EPathFollowingStatus::Moving;
}

float UMBaseTask::GetDistanceTo(AActor* Target) const
{
	if (!OwnerPawn || !Target) return MAX_FLT;
	return FVector::Dist(OwnerPawn->GetActorLocation(), Target->GetActorLocation());
}

float UMBaseTask::GetDistanceToLocation(const FVector& Location) const
{
	if (!OwnerPawn) return MAX_FLT;
	return FVector::Dist(OwnerPawn->GetActorLocation(), Location);
}
