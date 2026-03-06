// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MHoldPositionTask.generated.h"

class AMEveCharacter;
class AMOrdoCharacter;

/**
 * UMHoldPositionTask — 위치 고수 전투 태스크.
 * 이동하지 않고 사거리 내 적만 공격.
 */
UCLASS()
class THELASTSHELTER_API UMHoldPositionTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMHoldPositionTask();

	float AttackRange = 150.f;
	float AttackRate = 1.5f;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;

private:
	float LastAttackTime = -999.f;

	AActor* ResolveTarget() const;
	bool IsTargetDead(AActor* Target) const;
};
