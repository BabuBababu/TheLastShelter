// Copyright TheLastShelter. All Rights Reserved.

#include "MShelterInventoryManager.h"
#include "MDataManager.h"
#include "Engine/GameInstance.h"

void UMShelterInventoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UMDataManager>();
	Super::Initialize(Collection);

	// 초기 슬롯 생성
	Slots.SetNum(InitialSlotCount);

	UE_LOG(LogTemp, Log, TEXT("[ShelterInv] Initialized with %d slots"), InitialSlotCount);
}

void UMShelterInventoryManager::Deinitialize()
{
	Slots.Empty();
	Super::Deinitialize();
}

// ============================================================
// 아이템 추가
// ============================================================

int32 UMShelterInventoryManager::AddItem(int32 ItemId, int32 Count)
{
	if (Count <= 0 || ItemId <= 0) return Count;

	const int32 maxStack = GetMaxStack(ItemId);
	int32 remaining = Count;

	// 1) 기존 슬롯에 스택 가능한 곳 우선
	for (FMInventorySlot& slot : Slots)
	{
		if (remaining <= 0) break;
		if (slot.ItemId == ItemId && slot.Count < maxStack)
		{
			int32 canAdd = FMath::Min(remaining, maxStack - slot.Count);
			slot.Count += canAdd;
			remaining -= canAdd;
		}
	}

	// 2) 빈 슬롯에 배치
	for (FMInventorySlot& slot : Slots)
	{
		if (remaining <= 0) break;
		if (slot.IsEmpty())
		{
			slot.ItemId = ItemId;
			int32 canAdd = FMath::Min(remaining, maxStack);
			slot.Count = canAdd;
			remaining -= canAdd;
		}
	}

	if (remaining < Count)
	{
		OnShelterInventoryChanged.Broadcast();
	}

	return remaining;
}

// ============================================================
// 아이템 제거
// ============================================================

int32 UMShelterInventoryManager::RemoveItem(int32 ItemId, int32 Count)
{
	if (Count <= 0 || ItemId <= 0) return 0;

	int32 removed = 0;

	for (FMInventorySlot& slot : Slots)
	{
		if (removed >= Count) break;
		if (slot.ItemId == ItemId)
		{
			int32 toRemove = FMath::Min(Count - removed, slot.Count);
			slot.Count -= toRemove;
			removed += toRemove;

			if (slot.Count <= 0)
			{
				slot.ItemId = 0;
				slot.Count = 0;
			}
		}
	}

	if (removed > 0)
	{
		OnShelterInventoryChanged.Broadcast();
	}

	return removed;
}

// ============================================================
// 조회
// ============================================================

int32 UMShelterInventoryManager::GetItemCount(int32 ItemId) const
{
	int32 total = 0;
	for (const FMInventorySlot& slot : Slots)
	{
		if (slot.ItemId == ItemId)
		{
			total += slot.Count;
		}
	}
	return total;
}

TArray<FMInventorySlot> UMShelterInventoryManager::GetSlotsByCategory(EMShelterStorageCategory Category) const
{
	TArray<FMInventorySlot> result;

	for (const FMInventorySlot& slot : Slots)
	{
		if (slot.IsEmpty()) continue;
		if (GetItemCategory(slot.ItemId) == Category)
		{
			result.Add(slot);
		}
	}

	return result;
}

int32 UMShelterInventoryManager::GetEmptySlotCount() const
{
	int32 count = 0;
	for (const FMInventorySlot& slot : Slots)
	{
		if (slot.IsEmpty()) count++;
	}
	return count;
}

// ============================================================
// 가치 계산
// ============================================================

float UMShelterInventoryManager::CalculateTotalValue() const
{
	UMDataManager* dataMgr = GetGameInstance()->GetSubsystem<UMDataManager>();
	if (!dataMgr) return 0.f;

	float totalValue = 0.f;

	for (const FMInventorySlot& slot : Slots)
	{
		if (slot.IsEmpty()) continue;

		FMItemData itemData;
		if (dataMgr->GetItemDataByID(slot.ItemId, itemData))
		{
			totalValue += itemData.Value * slot.Count;
		}
	}

	return totalValue;
}

float UMShelterInventoryManager::CalculateValueByCategory(EMShelterStorageCategory Category) const
{
	UMDataManager* dataMgr = GetGameInstance()->GetSubsystem<UMDataManager>();
	if (!dataMgr) return 0.f;

	float totalValue = 0.f;

	for (const FMInventorySlot& slot : Slots)
	{
		if (slot.IsEmpty()) continue;

		FMItemData itemData;
		if (dataMgr->GetItemDataByID(slot.ItemId, itemData))
		{
			if (MapItemTypeToCategory(itemData.ItemType) == Category)
			{
				totalValue += itemData.Value * slot.Count;
			}
		}
	}

	return totalValue;
}

// ============================================================
// 용량 관리
// ============================================================

void UMShelterInventoryManager::ExpandCapacity(int32 AdditionalSlots)
{
	if (AdditionalSlots <= 0) return;

	int32 newTotal = Slots.Num() + AdditionalSlots;
	Slots.SetNum(newTotal);

	UE_LOG(LogTemp, Log, TEXT("[ShelterInv] Capacity expanded → %d slots (+%d)"),
		newTotal, AdditionalSlots);
}

void UMShelterInventoryManager::SetCapacity(int32 NewSlotCount)
{
	if (NewSlotCount <= 0) return;

	// 축소 시 기존 아이템은 보존 (빈 슬롯만 제거)
	if (NewSlotCount < Slots.Num())
	{
		// 뒤에서부터 빈 슬롯 제거
		while (Slots.Num() > NewSlotCount)
		{
			if (Slots.Last().IsEmpty())
			{
				Slots.RemoveAt(Slots.Num() - 1);
			}
			else
			{
				break; // 아이템이 있는 슬롯은 보존
			}
		}
	}
	else
	{
		Slots.SetNum(NewSlotCount);
	}

	UE_LOG(LogTemp, Log, TEXT("[ShelterInv] Capacity set → %d slots"), Slots.Num());
}

// ============================================================
// 내부 헬퍼
// ============================================================

int32 UMShelterInventoryManager::GetMaxStack(int32 ItemId) const
{
	UMDataManager* dataMgr = GetGameInstance()->GetSubsystem<UMDataManager>();
	if (dataMgr)
	{
		FMItemData itemData;
		if (dataMgr->GetItemDataByID(ItemId, itemData))
		{
			return FMath::Max(1, itemData.MaxStack);
		}
	}
	return 1;
}

EMShelterStorageCategory UMShelterInventoryManager::MapItemTypeToCategory(EMItemType ItemType) const
{
	switch (ItemType)
	{
		case EMItemType::Survival:		return EMShelterStorageCategory::Resource;
		case EMItemType::Consumable:	return EMShelterStorageCategory::Consumable;
		case EMItemType::Tool:			return EMShelterStorageCategory::General;
		case EMItemType::Equipment:		return EMShelterStorageCategory::Equipment;
		case EMItemType::Building:		return EMShelterStorageCategory::Building;
		case EMItemType::Misc:			return EMShelterStorageCategory::General;
		default:						return EMShelterStorageCategory::General;
	}
}

EMShelterStorageCategory UMShelterInventoryManager::GetItemCategory(int32 ItemId) const
{
	UMDataManager* dataMgr = GetGameInstance()->GetSubsystem<UMDataManager>();
	if (dataMgr)
	{
		FMItemData itemData;
		if (dataMgr->GetItemDataByID(ItemId, itemData))
		{
			return MapItemTypeToCategory(itemData.ItemType);
		}
	}
	return EMShelterStorageCategory::General;
}
