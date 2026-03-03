// Copyright TheLastShelter. All Rights Reserved.

#include "MShelterValueManager.h"
#include "MDataManager.h"
#include "MShelterInventoryManager.h"
#include "MStatComponent.h"
#include "Kismet/GameplayStatics.h"

void UMShelterValueManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 종속 서브시스템 초기화 보장
	Collection.InitializeDependency<UMDataManager>();
	Collection.InitializeDependency<UMShelterInventoryManager>();

	// 쉘터 인벤토리 변경 구독 → 자동 재계산
	UMShelterInventoryManager* shelterInv = GetGameInstance()->GetSubsystem<UMShelterInventoryManager>();
	if (shelterInv)
	{
		shelterInv->OnShelterInventoryChanged.AddDynamic(this, &UMShelterValueManager::OnShelterInventoryChanged);
	}

	UE_LOG(LogTemp, Log, TEXT("[ShelterValue] Initialized"));
}

void UMShelterValueManager::Deinitialize()
{
	RegisteredEveStats.Empty();
	Super::Deinitialize();
}

// ============================================================
// 기지 가치 계산
// ============================================================

float UMShelterValueManager::RecalculateBaseValue()
{
	float evePower = CalculateEveCombatPower();
	float inventoryValue = CalculateInventoryValue();

	float newValue = evePower + inventoryValue;

	if (!FMath::IsNearlyEqual(CachedBaseValue, newValue, 0.1f))
	{
		CachedBaseValue = newValue;
		OnBaseValueChanged.Broadcast(CachedBaseValue);
		UE_LOG(LogTemp, Log, TEXT("[ShelterValue] BaseValue=%.1f (Eve=%.1f + Inv=%.1f)"),
			CachedBaseValue, evePower, inventoryValue);
	}

	return CachedBaseValue;
}

float UMShelterValueManager::CalculateEveCombatPower() const
{
	float totalPower = 0.f;

	for (const TWeakObjectPtr<UMStatComponent>& weakStat : RegisteredEveStats)
	{
		if (UMStatComponent* statComp = weakStat.Get())
		{
			if (!statComp->IsDead())
			{
				totalPower += statComp->CalculateCombatPower();
			}
		}
	}

	return totalPower;
}

float UMShelterValueManager::CalculateInventoryValue() const
{
	UMShelterInventoryManager* shelterInv = GetGameInstance()->GetSubsystem<UMShelterInventoryManager>();
	if (!shelterInv) return 0.f;

	return shelterInv->CalculateTotalValue();
}

// ============================================================
// 쉘터 인벤토리 변경 콜백
// ============================================================

void UMShelterValueManager::OnShelterInventoryChanged()
{
	RecalculateBaseValue();
}

// ============================================================
// Eve 등록/해제
// ============================================================

void UMShelterValueManager::RegisterEveStatComponent(UMStatComponent* StatComp)
{
	if (!StatComp) return;

	// 중복 방지
	for (const TWeakObjectPtr<UMStatComponent>& existing : RegisteredEveStats)
	{
		if (existing.Get() == StatComp) return;
	}

	RegisteredEveStats.Add(StatComp);
	RecalculateBaseValue();
}

void UMShelterValueManager::UnregisterEveStatComponent(UMStatComponent* StatComp)
{
	RegisteredEveStats.RemoveAll([StatComp](const TWeakObjectPtr<UMStatComponent>& Weak)
	{
		return !Weak.IsValid() || Weak.Get() == StatComp;
	});

	RecalculateBaseValue();
}
