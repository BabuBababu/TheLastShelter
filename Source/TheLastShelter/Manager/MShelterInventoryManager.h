// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MShelterInventoryManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShelterInventoryChanged);

class UMDataManager;

/**
 * MShelterInventoryManager
 *
 * 쉘터(기지) 전용 인벤토리 매니저.
 * MInventoryManager(캐릭터 인벤토리)와 분리된 독립 서브시스템.
 *
 * 차이점:
 * - 카테고리 기반 저장 (EMShelterStorageCategory)
 * - 용량 업그레이드 가능 (UpgradeCapacity)
 * - 아이템 총 가치 직접 계산
 * - 싱글턴 (OwnerID 없음)
 */
UCLASS()
class THELASTSHELTER_API UMShelterInventoryManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================
	// 아이템 추가 / 제거
	// ============================================================

	/**
	 * 쉘터에 아이템 추가. 카테고리는 EMItemType에서 자동 매핑.
	 * @return 남은 수량 (0이면 전부 추가됨)
	 */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory")
	int32 AddItem(const FString& ItemID, int32 Count = 1);

	/** 쉘터에서 아이템 제거. @return 실제 제거된 수량 */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory")
	int32 RemoveItem(const FString& ItemID, int32 Count = 1);

	// ============================================================
	// 조회
	// ============================================================

	/** 특정 아이템 보유 수량 */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory")
	int32 GetItemCount(const FString& ItemID) const;

	/** 전체 슬롯 조회 */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory")
	TArray<FMInventorySlot> GetAllSlots() const { return Slots; }

	/** 특정 카테고리 슬롯만 조회 */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory")
	TArray<FMInventorySlot> GetSlotsByCategory(EMShelterStorageCategory Category) const;

	/** 빈 슬롯 수 */
	UFUNCTION(BlueprintPure, Category = "ShelterInventory")
	int32 GetEmptySlotCount() const;

	/** 전체 슬롯 수 */
	UFUNCTION(BlueprintPure, Category = "ShelterInventory")
	int32 GetTotalSlotCount() const { return Slots.Num(); }

	// ============================================================
	// 가치 계산
	// ============================================================

	/** 쉘터 인벤토리 아이템 총 가치 */
	UFUNCTION(BlueprintPure, Category = "ShelterInventory|Value")
	float CalculateTotalValue() const;

	/** 특정 카테고리 아이템 가치 합산 */
	UFUNCTION(BlueprintPure, Category = "ShelterInventory|Value")
	float CalculateValueByCategory(EMShelterStorageCategory Category) const;

	// ============================================================
	// 용량 관리
	// ============================================================

	/** 슬롯 수 추가 (건물 업그레이드 시) */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory|Capacity")
	void ExpandCapacity(int32 AdditionalSlots);

	/** 현재 최대 슬롯 수 설정 */
	UFUNCTION(BlueprintCallable, Category = "ShelterInventory|Capacity")
	void SetCapacity(int32 NewSlotCount);

	// ============================================================
	// 이벤트
	// ============================================================

	/** 쉘터 인벤토리 내용이 변경될 때 */
	UPROPERTY(BlueprintAssignable, Category = "ShelterInventory|Event")
	FOnShelterInventoryChanged OnShelterInventoryChanged;

	// ============================================================
	// 설정
	// ============================================================

	/** 초기 슬롯 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShelterInventory|Config")
	int32 InitialSlotCount = 50;

private:
	/** 인벤토리 슬롯 배열 */
	UPROPERTY()
	TArray<FMInventorySlot> Slots;

	/** 아이템 최대 스택 조회 (MDataManager 위임) */
	int32 GetMaxStack(const FString& ItemID) const;

	/** EMItemType → EMShelterStorageCategory 매핑 */
	EMShelterStorageCategory MapItemTypeToCategory(EMItemType ItemType) const;

	/** 특정 아이템이 속하는 카테고리 조회 */
	EMShelterStorageCategory GetItemCategory(const FString& ItemID) const;
};
