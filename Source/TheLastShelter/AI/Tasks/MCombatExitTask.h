// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MCombatExitTask.generated.h"

/**
 * UMCombatExitTask — 전투 해제(CombatExit) 전용 태스크.
 *
 * 전투 시작 애니메이션을 역재생하고, 완료 후 GunIdle로 복원.
 * Attack 태스크가 외부에서 중단되었을 때 깔끔한 전투 해제를 보장.
 *
 * 사용 시나리오:
 *   1) Attack 태스크가 InterruptWithTask로 취소된 후, 
 *      컨트롤러가 MCombatExitTask를 끼워넣어 역재생 후 다음 태스크 진행.
 *   2) HoldPosition 등 다른 전투 태스크 해제 시에도 사용 가능.
 */
UCLASS()
class THELASTSHELTER_API UMCombatExitTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMCombatExitTask();

	/** CombatExit 역재생 지속 시간 (초) — CombatEnterDuration과 동일하게 설정 */
	float Duration = 0.6f;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void EndTask(bool bWasCancelled) override;

private:
	float PhaseStartTime = 0.f;

	/** Eve/Ordo에 맞는 CombatExit 애니메이션 적용 */
	void ApplyCombatExitAnim();

	/** PlayRate 복원 + GunIdle 설정 */
	void RestoreToIdle();
};
