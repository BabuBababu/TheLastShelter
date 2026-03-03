// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MWorldTimeManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDayChanged, int32, NewDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeOfDayChanged, float, NormalizedTime, int32, CurrentDay);

/**
 * MWorldTimeManager
 * 인게임 하루 시간 흐름과 경과 일수를 관리합니다.
 *
 * - NormalizedTime (0~1): 하루 진행도  
 * - CurrentDay: 게임 시작 후 경과 일수
 * - DayLengthSeconds: 실시간 기준 하루 길이(초)
 *
 * Tick은 World의 DeltaTime을 사용하므로,
 * GameMode/PlayerController에서 매 프레임 AdvanceTime()을 호출해야 합니다.
 */
UCLASS()
class THELASTSHELTER_API UMWorldTimeManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ============================================================
	// 시간 진행
	// ============================================================

	/** 게임 틱마다 호출. DeltaTime을 받아 시간을 진행시킵니다. */
	UFUNCTION(BlueprintCallable, Category = "WorldTime")
	void AdvanceTime(float DeltaSeconds);

	// ============================================================
	// 조회
	// ============================================================

	/** 현재 일수 (0부터 시작) */
	UFUNCTION(BlueprintPure, Category = "WorldTime")
	int32 GetCurrentDay() const { return CurrentDay; }

	/** 하루 내 정규화 시간 (0.0=자정 ~ 1.0=다음 자정) */
	UFUNCTION(BlueprintPure, Category = "WorldTime")
	float GetNormalizedTime() const { return NormalizedTime; }

	/** 현재 시간대 */
	UFUNCTION(BlueprintPure, Category = "WorldTime")
	EMTimeOfDay GetCurrentTimeOfDay() const;

	/** 실시간 기준 하루 길이 (초) */
	UFUNCTION(BlueprintPure, Category = "WorldTime")
	float GetDayLengthSeconds() const { return DayLengthSeconds; }

	/** 시간 배율 (Pause=0, Fast=2 등) */
	UFUNCTION(BlueprintPure, Category = "WorldTime")
	float GetTimeScale() const { return TimeScale; }

	// ============================================================
	// 설정
	// ============================================================

	/** 실시간 기준 하루 길이 설정 */
	UFUNCTION(BlueprintCallable, Category = "WorldTime")
	void SetDayLengthSeconds(float InSeconds);

	/** 시간 배율 설정 (0=일시정지) */
	UFUNCTION(BlueprintCallable, Category = "WorldTime")
	void SetTimeScale(float InScale);

	/** 특정 시간으로 강제 이동 */
	UFUNCTION(BlueprintCallable, Category = "WorldTime")
	void SetTime(int32 Day, float InNormalizedTime);

	// ============================================================
	// 이벤트
	// ============================================================

	/** 날이 바뀔 때 */
	UPROPERTY(BlueprintAssignable, Category = "WorldTime|Event")
	FOnDayChanged OnDayChanged;

	/** NormalizedTime이 갱신될 때 (매 프레임) */
	UPROPERTY(BlueprintAssignable, Category = "WorldTime|Event")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

protected:
	/** 실시간 기준 하루 길이 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldTime")
	float DayLengthSeconds = 600.f;  // 10분 = 1일

	/** 시간 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldTime")
	float TimeScale = 1.f;

	/** 현재 경과 일수 */
	UPROPERTY(BlueprintReadOnly, Category = "WorldTime")
	int32 CurrentDay = 0;

	/** 하루 내 정규화 시간 (0~1) */
	UPROPERTY(BlueprintReadOnly, Category = "WorldTime")
	float NormalizedTime = 0.25f;  // 기본 시작: 새벽→낮 전환 시점
};
