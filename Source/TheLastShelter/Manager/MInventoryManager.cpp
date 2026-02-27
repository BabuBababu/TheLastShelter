// Copyright TheLastShelter. All Rights Reserved.

#include "MInventoryManager.h"
#include "MDataManager.h"
#include "Engine/GameInstance.h"

void UMInventoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 플레이어 인벤토리는 항상 생성
	CreateInventory(TEXT("Player"), 30);

	UE_LOG(LogTemp, Log, TEXT("[MInventoryManager] Initialized. Player inventory created."));
}

void UMInventoryManager::CreateInventory(const FString& OwnerID, int32 SlotCount)
{
	if (InventoryMap.Contains(OwnerID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MInventoryManager] Inventory already exists for: %s"), *OwnerID);
		return;
	}

	TArray<FMInventorySlot> Slots;
	Slots.SetNum(SlotCount);
	InventoryMap.Add(OwnerID, MoveTemp(Slots));

	UE_LOG(LogTemp, Log, TEXT("[MInventoryManager] Created inventory for '%s' with %d slots"), *OwnerID, SlotCount);
}

void UMInventoryManager::RemoveInventory(const FString& OwnerID)
{
	if (InventoryMap.Remove(OwnerID) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[MInventoryManager] Removed inventory for: %s"), *OwnerID);
	}
}

int32 UMInventoryManager::AddItem(const FString& OwnerID, const FString& ItemID, int32 Count)
{
	TArray<FMInventorySlot>* Inventory = InventoryMap.Find(OwnerID);
	if (!Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MInventoryManager] No inventory for: %s"), *OwnerID);
		return Count;
	}

	const int32 MaxStack = GetMaxStack(ItemID);
	int32 Remaining = Count;

	// 1) 기존 슬롯에 스택 가능한 곳 우선 채우기
	for (FMInventorySlot& Slot : *Inventory)
	{
		if (Remaining <= 0) break;
		if (Slot.ItemID == ItemID && Slot.Count < MaxStack)
		{
			const int32 CanAdd = FMath::Min(Remaining, MaxStack - Slot.Count);
			Slot.Count += CanAdd;
			Remaining -= CanAdd;
		}
	}

	// 2) 빈 슬롯에 배치
	for (FMInventorySlot& Slot : *Inventory)
	{
		if (Remaining <= 0) break;
		if (Slot.IsEmpty())
		{
			Slot.ItemID = ItemID;
			const int32 CanAdd = FMath::Min(Remaining, MaxStack);
			Slot.Count = CanAdd;
			Remaining -= CanAdd;
		}
	}

	return Remaining; // 0이면 전량 추가 성공
}

int32 UMInventoryManager::RemoveItem(const FString& OwnerID, const FString& ItemID, int32 Count)
{
	TArray<FMInventorySlot>* Inventory = InventoryMap.Find(OwnerID);
	if (!Inventory) return 0;

	int32 Removed = 0;

	for (FMInventorySlot& Slot : *Inventory)
	{
		if (Removed >= Count) break;
		if (Slot.ItemID == ItemID)
		{
			const int32 ToRemove = FMath::Min(Count - Removed, Slot.Count);
			Slot.Count -= ToRemove;
			Removed += ToRemove;

			if (Slot.Count <= 0)
			{
				Slot.ItemID.Empty();
				Slot.Count = 0;
			}
		}
	}

	return Removed;
}

int32 UMInventoryManager::GetItemCount(const FString& OwnerID, const FString& ItemID) const
{
	const TArray<FMInventorySlot>* Inventory = InventoryMap.Find(OwnerID);
	if (!Inventory) return 0;

	int32 Total = 0;
	for (const FMInventorySlot& Slot : *Inventory)
	{
		if (Slot.ItemID == ItemID)
			Total += Slot.Count;
	}
	return Total;
}

TArray<FMInventorySlot> UMInventoryManager::GetInventory(const FString& OwnerID) const
{
	const TArray<FMInventorySlot>* Inventory = InventoryMap.Find(OwnerID);
	if (Inventory) return *Inventory;
	return TArray<FMInventorySlot>();
}

bool UMInventoryManager::HasInventory(const FString& OwnerID) const
{
	return InventoryMap.Contains(OwnerID);
}

int32 UMInventoryManager::GetEmptySlotCount(const FString& OwnerID) const
{
	const TArray<FMInventorySlot>* Inventory = InventoryMap.Find(OwnerID);
	if (!Inventory) return 0;

	int32 EmptyCount = 0;
	for (const FMInventorySlot& Slot : *Inventory)
	{
		if (Slot.IsEmpty()) EmptyCount++;
	}
	return EmptyCount;
}

int32 UMInventoryManager::GetMaxStack(const FString& ItemID) const
{
	UMDataManager* DataMgr = GetGameInstance()->GetSubsystem<UMDataManager>();
	if (DataMgr)
	{
		FMItemData ItemData;
		if (DataMgr->GetItemDataByID(ItemID, ItemData))
		{
			return FMath::Max(1, ItemData.MaxStack);
		}
	}
	return 1;
}
