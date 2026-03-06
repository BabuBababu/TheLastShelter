// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MRetreatTask.generated.h"

/**
 * UMRetreatTask — 퇴각 태스크.
 *
 * Ordo: TargetLocation(또는 RetreatDestination)으로 이동 후 디스폰.
 * Eve: 숙소(ShelterLocation)로 이동 후 완료.
 */
UCLASS()
class THELASTSHELTER_API UMRetreatTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMRetreatTask();

	/** 도착 판정 거리 */
	float ArrivalDistance = 100.f;

	/** 도착 시 Pawn 파괴 여부 (Ordo Retreat 시 true) */
	bool bDestroyOnArrival = false;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;
};
