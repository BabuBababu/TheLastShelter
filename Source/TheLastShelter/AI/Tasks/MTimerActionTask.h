// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MTimerActionTask.generated.h"

/**
 * UMTimerActionTask — 타이머 기반 행동 태스크.
 * 이동 정지 → 애니메이션 설정 → 지정 시간 대기 → 완료.
 * Rest, Sex, Masturbation 등에 공통 사용.
 */
UCLASS()
class THELASTSHELTER_API UMTimerActionTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMTimerActionTask();

	/** 행동 지속 시간 (초) */
	float Duration = 5.0f;

	/** Eve 애니메이션 상태 (Eve 소유 시) */
	EMEveAnimState EveAnimState = EMEveAnimState::SitRest;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;
};
