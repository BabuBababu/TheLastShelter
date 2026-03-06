// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MCarryTask.generated.h"

/**
 * UMCarryTask — 운반 태스크.
 * 대상 액터 위치로 이동 → 도착 시 운반 애니메이션 → 완료.
 */
UCLASS()
class THELASTSHELTER_API UMCarryTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMCarryTask();

	float PickupDistance = 80.f;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;
};
