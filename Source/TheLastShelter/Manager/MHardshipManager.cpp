// Copyright TheLastShelter. All Rights Reserved.

#include "MHardshipManager.h"
#include "MWorldTimeManager.h"
#include "MShelterValueManager.h"
#include "MSpawnManager.h"
#include "MDataManager.h"

void UMHardshipManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UMWorldTimeManager>();
	Collection.InitializeDependency<UMShelterValueManager>();
	Collection.InitializeDependency<UMDataManager>();

	// OnDayChanged 구독
	UMWorldTimeManager* timeMgr = GetGameInstance()->GetSubsystem<UMWorldTimeManager>();
	if (timeMgr)
	{
		timeMgr->OnDayChanged.AddDynamic(this, &UMHardshipManager::OnDayChanged);
	}

	// 기본 재난 정의 (비어 있을 때 기본값 세팅)
	if (HardshipDefinitions.Num() == 0)
	{
		// 오르도 침공
		FMHardshipEventData invasion;
		invasion.HardshipType = EMHardshipType::OrdoInvasion;
		invasion.BaseProbability = 0.3f;
		invasion.ProbabilityPerBaseValue = 0.001f;
		invasion.MinDay = 1;
		invasion.DurationDays = 0; // 즉시 (웨이브 완료 시 종료)

		FMWaveDefinition defaultWave;
		defaultWave.MinBaseValue = 0.f;
		defaultWave.MinSpawnCount = 2;
		defaultWave.MaxSpawnCount = 5;
		defaultWave.ExtraSpawnPerBaseValue = 0.01f;
		defaultWave.WaveInterval = 10.f;
		invasion.InvasionWaves.Add(defaultWave);
		HardshipDefinitions.Add(invasion);

		// 전염병
		FMHardshipEventData plague;
		plague.HardshipType = EMHardshipType::Plague;
		plague.BaseProbability = 0.05f;
		plague.ProbabilityPerBaseValue = 0.0005f;
		plague.MinDay = 5;
		plague.DurationDays = 3;
		plague.MinBaseValue = 50.f;
		HardshipDefinitions.Add(plague);

		// 장마
		FMHardshipEventData monsoon;
		monsoon.HardshipType = EMHardshipType::Monsoon;
		monsoon.BaseProbability = 0.1f;
		monsoon.ProbabilityPerBaseValue = 0.0003f;
		monsoon.MinDay = 3;
		monsoon.DurationDays = 5;
		HardshipDefinitions.Add(monsoon);

		// 태풍
		FMHardshipEventData typhoon;
		typhoon.HardshipType = EMHardshipType::Typhoon;
		typhoon.BaseProbability = 0.08f;
		typhoon.ProbabilityPerBaseValue = 0.0004f;
		typhoon.MinDay = 7;
		typhoon.DurationDays = 2;
		typhoon.MinBaseValue = 100.f;
		HardshipDefinitions.Add(typhoon);

		// 폭설
		FMHardshipEventData blizzard;
		blizzard.HardshipType = EMHardshipType::Blizzard;
		blizzard.BaseProbability = 0.07f;
		blizzard.ProbabilityPerBaseValue = 0.0003f;
		blizzard.MinDay = 10;
		blizzard.DurationDays = 3;
		blizzard.MinBaseValue = 80.f;
		HardshipDefinitions.Add(blizzard);

		// 한파
		FMHardshipEventData coldWave;
		coldWave.HardshipType = EMHardshipType::ColdWave;
		coldWave.BaseProbability = 0.06f;
		coldWave.ProbabilityPerBaseValue = 0.0002f;
		coldWave.MinDay = 10;
		coldWave.DurationDays = 4;
		coldWave.MinBaseValue = 80.f;
		HardshipDefinitions.Add(coldWave);
	}

	UE_LOG(LogTemp, Log, TEXT("[HardshipManager] Initialized (%d definitions)"), HardshipDefinitions.Num());
}

void UMHardshipManager::Deinitialize()
{
	ActiveHardships.Empty();
	LastOccurrenceDay.Empty();
	Super::Deinitialize();
}

// ============================================================
// Tick
// ============================================================

void UMHardshipManager::TickHardship(float DeltaSeconds)
{
	// 현재는 일 단위 처리만 (OnDayChanged에서 EvaluateHardships 호출)
	// 추후 실시간 효과 (HP 지속 감소 등) 는 이 함수에서 DeltaSeconds 기반으로 적용
}

// ============================================================
// 매일 콜백
// ============================================================

void UMHardshipManager::OnDayChanged(int32 NewDay)
{
	// 1) 활성 재난의 하루 틱 (남은 일수 감소, 효과 적용)
	TickActiveHardshipsDaily(NewDay);

	// 2) 새 재난 평가
	EvaluateHardships(NewDay);
}

// ============================================================
// 재난 평가
// ============================================================

void UMHardshipManager::EvaluateHardships(int32 CurrentDay)
{
	// 동시 최대 수 제한
	if (ActiveHardships.Num() >= MaxSimultaneousHardships) return;

	UMShelterValueManager* shelterMgr = GetGameInstance()->GetSubsystem<UMShelterValueManager>();
	float baseValue = shelterMgr ? shelterMgr->GetBaseValue() : 0.f;

	UMWorldTimeManager* timeMgr = GetGameInstance()->GetSubsystem<UMWorldTimeManager>();
	EMTimeOfDay currentTimeOfDay = timeMgr ? timeMgr->GetCurrentTimeOfDay() : EMTimeOfDay::Day;

	for (const FMHardshipEventData& definition : HardshipDefinitions)
	{
		// 동시 최대 수 재확인
		if (ActiveHardships.Num() >= MaxSimultaneousHardships) break;

		// 최소 경과 일수 확인
		if (CurrentDay < definition.MinDay) continue;

		// 최소 기지 가치 확인
		if (baseValue < definition.MinBaseValue) continue;

		// 이미 같은 타입이 활성 중이면 스킵
		if (IsHardshipActive(definition.HardshipType)) continue;

		// 쿨다운 확인
		if (const int32* lastDay = LastOccurrenceDay.Find(definition.HardshipType))
		{
			if (CurrentDay - *lastDay < SameTypeCooldownDays) continue;
		}

		// 시간대 조건 확인
		if (definition.AllowedTimeOfDay.Num() > 0 &&
			!definition.AllowedTimeOfDay.Contains(currentTimeOfDay))
		{
			continue;
		}

		// 확률 계산
		float probability = definition.BaseProbability
			+ baseValue * definition.ProbabilityPerBaseValue;
		probability = FMath::Clamp(probability, 0.f, 0.95f);

		float roll = FMath::FRand();
		if (roll <= probability)
		{
			TriggerHardship(definition, baseValue);
		}
	}
}

// ============================================================
// 재난 시작
// ============================================================

void UMHardshipManager::TriggerHardship(const FMHardshipEventData& Definition, float BaseValue)
{
	EMHardshipSeverity severity = DetermineSeverity(BaseValue);

	FMHardshipEventData activeEvent = Definition;
	activeEvent.Severity = severity;
	activeEvent.DurationDays = CalculateDuration(Definition, severity);
	activeEvent.RemainingDays = activeEvent.DurationDays;

	UMWorldTimeManager* timeMgr = GetGameInstance()->GetSubsystem<UMWorldTimeManager>();
	int32 currentDay = timeMgr ? timeMgr->GetCurrentDay() : 0;
	LastOccurrenceDay.Add(Definition.HardshipType, currentDay);

	UE_LOG(LogTemp, Warning, TEXT("[Hardship] === %d STARTED (Severity=%d, Duration=%d days) ==="),
		static_cast<int32>(Definition.HardshipType),
		static_cast<int32>(severity),
		activeEvent.DurationDays);

	// 침공은 MSpawnManager에 위임
	if (Definition.HardshipType == EMHardshipType::OrdoInvasion)
	{
		ExecuteInvasion(activeEvent, BaseValue);
		// 침공은 MSpawnManager가 관리하므로 ActiveHardships에 추가하지만
		// DurationDays = 0이면 SpawnManager의 OnInvasionEnded에서 제거
	}

	ActiveHardships.Add(activeEvent);
	OnHardshipStarted.Broadcast(activeEvent);
}

void UMHardshipManager::ExecuteInvasion(const FMHardshipEventData& EventData, float BaseValue)
{
	UMSpawnManager* spawnMgr = GetGameInstance()->GetSubsystem<UMSpawnManager>();
	if (!spawnMgr) return;

	// 웨이브 정의를 SpawnManager에 전달하고 침공 시작
	spawnMgr->StartInvasionWithWaves(EventData.InvasionWaves);
}

// ============================================================
// 재난 종료
// ============================================================

void UMHardshipManager::EndHardship(int32 ActiveIndex)
{
	if (!ActiveHardships.IsValidIndex(ActiveIndex)) return;

	EMHardshipType endedType = ActiveHardships[ActiveIndex].HardshipType;

	UE_LOG(LogTemp, Log, TEXT("[Hardship] === %d ENDED ==="), static_cast<int32>(endedType));

	ActiveHardships.RemoveAt(ActiveIndex);
	OnHardshipEnded.Broadcast(endedType);
}

void UMHardshipManager::ForceStartHardship(EMHardshipType Type, EMHardshipSeverity Severity)
{
	// 정의 테이블에서 해당 타입 검색
	for (const FMHardshipEventData& definition : HardshipDefinitions)
	{
		if (definition.HardshipType == Type)
		{
			UMShelterValueManager* shelterMgr = GetGameInstance()->GetSubsystem<UMShelterValueManager>();
			float baseValue = shelterMgr ? shelterMgr->GetBaseValue() : 0.f;

			FMHardshipEventData modified = definition;
			modified.Severity = Severity;
			modified.DurationDays = CalculateDuration(definition, Severity);
			modified.RemainingDays = modified.DurationDays;

			if (Type == EMHardshipType::OrdoInvasion)
			{
				ExecuteInvasion(modified, baseValue);
			}

			ActiveHardships.Add(modified);
			OnHardshipStarted.Broadcast(modified);
			return;
		}
	}

	// 정의가 없어도 강제 발생 가능
	FMHardshipEventData forced;
	forced.HardshipType = Type;
	forced.Severity = Severity;
	forced.DurationDays = 3;
	forced.RemainingDays = 3;
	ActiveHardships.Add(forced);
	OnHardshipStarted.Broadcast(forced);
}

void UMHardshipManager::ForceEndHardship(EMHardshipType Type)
{
	for (int32 i = ActiveHardships.Num() - 1; i >= 0; --i)
	{
		if (ActiveHardships[i].HardshipType == Type)
		{
			// 침공이면 SpawnManager 종료도 함께
			if (Type == EMHardshipType::OrdoInvasion)
			{
				UMSpawnManager* spawnMgr = GetGameInstance()->GetSubsystem<UMSpawnManager>();
				if (spawnMgr) spawnMgr->ForceEndInvasion();
			}

			EndHardship(i);
			return;
		}
	}
}

void UMHardshipManager::ForceEndAllHardships()
{
	for (int32 i = ActiveHardships.Num() - 1; i >= 0; --i)
	{
		if (ActiveHardships[i].HardshipType == EMHardshipType::OrdoInvasion)
		{
			UMSpawnManager* spawnMgr = GetGameInstance()->GetSubsystem<UMSpawnManager>();
			if (spawnMgr) spawnMgr->ForceEndInvasion();
		}
		EndHardship(i);
	}
}

// ============================================================
// 지속형 재난 하루 틱
// ============================================================

void UMHardshipManager::TickActiveHardshipsDaily(int32 CurrentDay)
{
	for (int32 i = ActiveHardships.Num() - 1; i >= 0; --i)
	{
		FMHardshipEventData& active = ActiveHardships[i];

		// 침공(DurationDays=0)은 SpawnManager 종료 시까지 유지
		if (active.HardshipType == EMHardshipType::OrdoInvasion)
		{
			UMSpawnManager* spawnMgr = GetGameInstance()->GetSubsystem<UMSpawnManager>();
			if (spawnMgr && !spawnMgr->IsInvasionActive())
			{
				EndHardship(i);
			}
			continue;
		}

		// 지속형 재난: 남은 일수 감소
		active.RemainingDays--;
		OnHardshipDayTick.Broadcast(active.HardshipType, active.RemainingDays);

		UE_LOG(LogTemp, Log, TEXT("[Hardship] %d remaining: %d days"),
			static_cast<int32>(active.HardshipType), active.RemainingDays);

		// TODO: 재난 타입별 매일 효과 적용
		// - Plague: Eve/Player HP 감소
		// - Monsoon/Typhoon: 이동/작업 속도 디버프
		// - Blizzard/ColdWave: 스태미나 감소, 작업 불가
		// - Heatwave/Drought: 체력 감소, 자원 수확량 감소

		if (active.RemainingDays <= 0)
		{
			EndHardship(i);
		}
	}
}

// ============================================================
// 조회
// ============================================================

bool UMHardshipManager::IsHardshipActive(EMHardshipType Type) const
{
	for (const FMHardshipEventData& active : ActiveHardships)
	{
		if (active.HardshipType == Type) return true;
	}
	return false;
}

int32 UMHardshipManager::GetRemainingDays(EMHardshipType Type) const
{
	for (const FMHardshipEventData& active : ActiveHardships)
	{
		if (active.HardshipType == Type) return active.RemainingDays;
	}
	return 0;
}

// ============================================================
// 헬퍼
// ============================================================

float UMHardshipManager::GetSeverityMultiplier(EMHardshipSeverity Severity) const
{
	switch (Severity)
	{
		case EMHardshipSeverity::Minor:			return 0.5f;
		case EMHardshipSeverity::Moderate:		return 1.0f;
		case EMHardshipSeverity::Severe:		return 1.5f;
		case EMHardshipSeverity::Catastrophic:	return 2.5f;
		default:								return 1.0f;
	}
}

int32 UMHardshipManager::CalculateDuration(const FMHardshipEventData& Definition, EMHardshipSeverity Severity) const
{
	if (Definition.DurationDays <= 0) return 0; // 즉시형 (침공)

	float multiplier = GetSeverityMultiplier(Severity);
	int32 baseDuration = Definition.DurationDays;
	int32 result = FMath::RoundToInt(baseDuration * multiplier);
	return FMath::Max(1, result);
}

EMHardshipSeverity UMHardshipManager::DetermineSeverity(float BaseValue) const
{
	// 기지 가치 구간에 따라 심각도 결정 (랜덤 가중)
	float roll = FMath::FRand();

	if (BaseValue >= 500.f)
	{
		// 고가치: Catastrophic 10%, Severe 30%, Moderate 40%, Minor 20%
		if (roll < 0.10f)		return EMHardshipSeverity::Catastrophic;
		if (roll < 0.40f)		return EMHardshipSeverity::Severe;
		if (roll < 0.80f)		return EMHardshipSeverity::Moderate;
		return EMHardshipSeverity::Minor;
	}
	else if (BaseValue >= 200.f)
	{
		// 중가치: Severe 15%, Moderate 45%, Minor 40%
		if (roll < 0.15f)		return EMHardshipSeverity::Severe;
		if (roll < 0.60f)		return EMHardshipSeverity::Moderate;
		return EMHardshipSeverity::Minor;
	}
	else
	{
		// 저가치: Moderate 30%, Minor 70%
		if (roll < 0.30f)		return EMHardshipSeverity::Moderate;
		return EMHardshipSeverity::Minor;
	}
}
