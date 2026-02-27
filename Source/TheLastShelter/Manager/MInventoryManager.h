// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MInventoryManager.generated.h"

/**
 * MInventoryManager
 * 플레이어 + 각 Eve 인스턴스의 인벤토리를 중앙 관리합니다.
 * OwnerID(FString)를 키로 사용하며, 플레이어는 "Player",
 * Eve는 EveData의 ID를 OwnerID로 사용합니다.
 */
UCLASS()
class THELASTSHELTER_API UMInventoryManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 소유자 인벤토리 생성 (슬롯 수 지정) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CreateInventory(const FString& OwnerID, int32 SlotCount = 20);

	/** 인벤토리 제거 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveInventory(const FString& OwnerID);

	/** 아이템 추가, 남은 수량 반환 (0이면 전부 추가됨) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(const FString& OwnerID, const FString& ItemID, int32 Count = 1);

	/** 아이템 제거, 실제 제거된 수량 반환 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(const FString& OwnerID, const FString& ItemID, int32 Count = 1);

	/** 특정 아이템 보유 수량 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(const FString& OwnerID, const FString& ItemID) const;

	/** 전체 인벤토리 슬롯 조회 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FMInventorySlot> GetInventory(const FString& OwnerID) const;

	/** 인벤토리 존재 여부 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasInventory(const FString& OwnerID) const;

	/** 빈 슬롯 수 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetEmptySlotCount(const FString& OwnerID) const;

private:
	/** OwnerID → 인벤토리 슬롯 배열 */
	TMap<FString, TArray<FMInventorySlot>> InventoryMap;

	/** 아이템의 최대 스택을 DataManager에서 가져오기 */
	int32 GetMaxStack(const FString& ItemID) const;
};
