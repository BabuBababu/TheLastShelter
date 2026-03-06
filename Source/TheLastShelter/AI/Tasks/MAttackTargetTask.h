// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MAttackTargetTask.generated.h"

/**
 * UMAttackTargetTask — 타겟 액터 공격 태스크 (DestroyStorage, AttackDefenseTower 등).
 * CombatEnter/Exit 없이 단순히 이동 → 사거리 내 공격 반복.
 */
UCLASS()
class THELASTSHELTER_API UMAttackTargetTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMAttackTargetTask();

	float AttackRange = 120.f;
	float AttackRate = 2.0f;

	/** 피격 무시 여부 (AttackDefenseTower는 true) */
	bool bIgnoreDamage = false;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;

private:
	float LastAttackTime = -999.f;
};
