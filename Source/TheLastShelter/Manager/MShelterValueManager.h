// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MShelterValueManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBaseValueChanged, float, NewBaseValue);

class UMDataManager;
class UMShelterInventoryManager;
class UMStatComponent;

/**
 * MShelterValueManager
 * 쉘터(기지)의 종합 가치를 산출하고 관리합니다.
 *
 * 기지 가치 = Eve들의 전투력 합 + MShelterInventoryManager의 아이템 가치 합.
 * MSpawnManager가 이 수치를 참조하여 침공 규모를 결정합니다.
 */
UCLASS()
class THELASTSHELTER_API UMShelterValueManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================
	// 기지 가치 조회
	// ============================================================

	/** 캐시된 기지 가치 반환 */
	UFUNCTION(BlueprintPure, Category = "Shelter")
	float GetBaseValue() const { return CachedBaseValue; }

	/** 기지 가치 재계산 (Eve 전투력 + 쉘터 인벤토리 가치) */
	UFUNCTION(BlueprintCallable, Category = "Shelter")
	float RecalculateBaseValue();

	// ============================================================
	// Eve 전투력 등록/해제
	// ============================================================

	/** Eve의 StatComponent를 등록 (스폰/초기화 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Shelter")
	void RegisterEveStatComponent(UMStatComponent* StatComp);

	/** Eve의 StatComponent를 해제 (사망/디스폰 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Shelter")
	void UnregisterEveStatComponent(UMStatComponent* StatComp);

	// ============================================================
	// 이벤트
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "Shelter|Event")
	FOnBaseValueChanged OnBaseValueChanged;

private:
	/** Eve들의 StatComponent 약참조 목록 */
	UPROPERTY()
	TArray<TWeakObjectPtr<UMStatComponent>> RegisteredEveStats;

	/** 캐시된 기지 가치 */
	float CachedBaseValue = 0.f;

	/** MShelterInventoryManager 인벤토리 변경 콜백 */
	UFUNCTION()
	void OnShelterInventoryChanged();

	/** 쉘터 인벤토리 아이템 가치 합산 */
	float CalculateInventoryValue() const;

	/** Eve 전투력 합산 */
	float CalculateEveCombatPower() const;
};
