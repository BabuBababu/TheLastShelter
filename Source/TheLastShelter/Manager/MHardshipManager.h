// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MHardshipManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHardshipStarted, const FMHardshipEventData&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHardshipEnded, EMHardshipType, EndedType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHardshipDayTick, EMHardshipType, Type, int32, RemainingDays);

class UMWorldTimeManager;
class UMShelterValueManager;
class UMSpawnManager;

/**
 * MHardshipManager
 *
 * 쉘터에 닥치는 모든 재난/역경 이벤트를 관리합니다.
 * MWorldTimeManager의 시간 흐름을 구독하고, MShelterValueManager의 기지 가치를
 * 참조하여 랜덤으로 재난을 발생시킵니다.
 *
 * 재난 타입:
 * - OrdoInvasion: MSpawnManager에 침공 명령 위임
 * - Plague: 캐릭터 HP/스태미나 지속 감소
 * - Monsoon/Typhoon: 이동속도 감소, 작업속도 감소
 * - Blizzard/ColdWave: 스태미나 지속 감소, 작업 불가
 * - Heatwave/Drought: 체력 지속 감소, 자원 수확량 감소
 *
 * 핵심 흐름:
 *   OnDayChanged → EvaluateHardships → TriggerHardship → (지속형은 TickActiveHardships)
 *                                     → EndHardship
 */
UCLASS()
class THELASTSHELTER_API UMHardshipManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================
	// 매 프레임 갱신
	// ============================================================

	/** GameMode/PlayerController에서 매 프레임 호출 */
	UFUNCTION(BlueprintCallable, Category = "Hardship")
	void TickHardship(float DeltaSeconds);

	// ============================================================
	// 재난 수동 제어
	// ============================================================

	/** 재난 강제 발생 (디버그/이벤트 스크립트용) */
	UFUNCTION(BlueprintCallable, Category = "Hardship")
	void ForceStartHardship(EMHardshipType Type, EMHardshipSeverity Severity = EMHardshipSeverity::Moderate);

	/** 특정 재난 강제 종료 */
	UFUNCTION(BlueprintCallable, Category = "Hardship")
	void ForceEndHardship(EMHardshipType Type);

	/** 모든 활성 재난 종료 */
	UFUNCTION(BlueprintCallable, Category = "Hardship")
	void ForceEndAllHardships();

	// ============================================================
	// 조회
	// ============================================================

	/** 특정 타입 재난이 활성 중인지 */
	UFUNCTION(BlueprintPure, Category = "Hardship")
	bool IsHardshipActive(EMHardshipType Type) const;

	/** 현재 활성 재난 목록 */
	UFUNCTION(BlueprintCallable, Category = "Hardship")
	TArray<FMHardshipEventData> GetActiveHardships() const { return ActiveHardships; }

	/** 활성 재난 수 */
	UFUNCTION(BlueprintPure, Category = "Hardship")
	int32 GetActiveHardshipCount() const { return ActiveHardships.Num(); }

	/** 특정 타입 재난의 남은 일수 */
	UFUNCTION(BlueprintPure, Category = "Hardship")
	int32 GetRemainingDays(EMHardshipType Type) const;

	// ============================================================
	// 재난 정의 (에디터/런타임 설정)
	// ============================================================

	/** 발생 가능한 재난 정의 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship|Config")
	TArray<FMHardshipEventData> HardshipDefinitions;

	/** 하루에 동시 발생 가능한 최대 재난 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship|Config")
	int32 MaxSimultaneousHardships = 2;

	/** 같은 재난 연속 발생 쿨다운 (일) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship|Config")
	int32 SameTypeCooldownDays = 3;

	// ============================================================
	// 이벤트
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "Hardship|Event")
	FOnHardshipStarted OnHardshipStarted;

	UPROPERTY(BlueprintAssignable, Category = "Hardship|Event")
	FOnHardshipEnded OnHardshipEnded;

	/** 지속형 재난의 매일 틱 (UI 갱신용) */
	UPROPERTY(BlueprintAssignable, Category = "Hardship|Event")
	FOnHardshipDayTick OnHardshipDayTick;

private:
	// ============================================================
	// 내부 로직
	// ============================================================

	/** OnDayChanged 콜백 */
	UFUNCTION()
	void OnDayChanged(int32 NewDay);

	/** 매일 재난 평가 */
	void EvaluateHardships(int32 CurrentDay);

	/** 재난 시작 */
	void TriggerHardship(const FMHardshipEventData& Definition, float BaseValue);

	/** 재난 종료 */
	void EndHardship(int32 ActiveIndex);

	/** 지속형 재난의 하루 틱 */
	void TickActiveHardshipsDaily(int32 CurrentDay);

	/** 침공 타입 재난 → MSpawnManager 위임 */
	void ExecuteInvasion(const FMHardshipEventData& EventData, float BaseValue);

	/** 심각도에 따른 확률 보정 배율 */
	float GetSeverityMultiplier(EMHardshipSeverity Severity) const;

	/** 심각도에 따른 지속일수 계산 */
	int32 CalculateDuration(const FMHardshipEventData& Definition, EMHardshipSeverity Severity) const;

	/** 기지 가치에 따른 심각도 결정 */
	EMHardshipSeverity DetermineSeverity(float BaseValue) const;

	// ============================================================
	// 내부 상태
	// ============================================================

	/** 현재 활성 재난 목록 */
	UPROPERTY()
	TArray<FMHardshipEventData> ActiveHardships;

	/** 재난타입 → 마지막 발생 일자 (쿨다운용) */
	TMap<EMHardshipType, int32> LastOccurrenceDay;
};
