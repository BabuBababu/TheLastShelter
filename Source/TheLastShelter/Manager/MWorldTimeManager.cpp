// Copyright TheLastShelter. All Rights Reserved.

#include "MWorldTimeManager.h"

void UMWorldTimeManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("[WorldTime] Initialized (DayLength=%.0fs, Start=Day%d T=%.2f)"),
		DayLengthSeconds, CurrentDay, NormalizedTime);
}

// ============================================================
// 시간 진행
// ============================================================

void UMWorldTimeManager::AdvanceTime(float DeltaSeconds)
{
	if (DayLengthSeconds <= 0.f || TimeScale <= 0.f) return;

	float timeDelta = (DeltaSeconds * TimeScale) / DayLengthSeconds;
	NormalizedTime += timeDelta;

	// 날짜 경과 처리
	while (NormalizedTime >= 1.f)
	{
		NormalizedTime -= 1.f;
		CurrentDay++;
		OnDayChanged.Broadcast(CurrentDay);

		UE_LOG(LogTemp, Log, TEXT("[WorldTime] Day changed → %d"), CurrentDay);
	}

	OnTimeOfDayChanged.Broadcast(NormalizedTime, CurrentDay);
}

// ============================================================
// 조회
// ============================================================

EMTimeOfDay UMWorldTimeManager::GetCurrentTimeOfDay() const
{
	if (NormalizedTime < 0.25f)		return EMTimeOfDay::Dawn;
	if (NormalizedTime < 0.50f)		return EMTimeOfDay::Day;
	if (NormalizedTime < 0.75f)		return EMTimeOfDay::Dusk;
	return EMTimeOfDay::Night;
}

// ============================================================
// 설정
// ============================================================

void UMWorldTimeManager::SetDayLengthSeconds(float InSeconds)
{
	DayLengthSeconds = FMath::Max(1.f, InSeconds);
}

void UMWorldTimeManager::SetTimeScale(float InScale)
{
	TimeScale = FMath::Max(0.f, InScale);
}

void UMWorldTimeManager::SetTime(int32 Day, float InNormalizedTime)
{
	CurrentDay = FMath::Max(0, Day);
	NormalizedTime = FMath::Clamp(InNormalizedTime, 0.f, 0.9999f);

	OnDayChanged.Broadcast(CurrentDay);
	OnTimeOfDayChanged.Broadcast(NormalizedTime, CurrentDay);
}
