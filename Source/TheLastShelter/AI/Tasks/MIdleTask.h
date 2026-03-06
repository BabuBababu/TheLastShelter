// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MIdleTask.generated.h"

/**
 * UMIdleTask — 대기/배회/순찰 태스크.
 *
 * Eve: 플레이어 근처에서 랜덤 배회.
 * Ordo: SpawnLocation 근처에서 순찰.
 *
 * 일정 시간 후 자동 완료 (다음 Idle 자동 재생성).
 */
UCLASS()
class THELASTSHELTER_API UMIdleTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMIdleTask();

	/** 배회/순찰 중심점 (Eve: 플레이어 위치, Ordo: 스폰 위치) */
	FVector WanderOrigin = FVector::ZeroVector;

	/** 배회 반경 */
	float WanderRadius = 250.f;

	/** 각 배회 대기 시간(초) */
	float WanderIdleTime = 2.0f;

	/** 전체 Idle 태스크 지속 시간(초) — 0이면 무제한 */
	float TotalDuration = 0.f;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;

private:
	enum class EIdlePhase : uint8
	{
		Waiting,    // 한 곳에서 대기
		Moving      // 새 배회 지점으로 이동
	};

	EIdlePhase CurrentPhase = EIdlePhase::Waiting;
	FVector WanderTarget = FVector::ZeroVector;
	float WaitEndTime = 0.f;

	void ChooseNewWanderPoint();
	void BeginWaiting();
};
