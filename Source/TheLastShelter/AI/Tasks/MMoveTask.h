// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MMoveTask.generated.h"

/**
 * UMMoveTask — 이동 태스크 (Move / ForceMove).
 * TargetActor 또는 TargetLocation까지 이동 후 완료.
 */
UCLASS()
class THELASTSHELTER_API UMMoveTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMMoveTask();

	/** 도착 판정 거리 */
	float ArrivalDistance = 50.f;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;

private:
	FVector GetDestination() const;
};
