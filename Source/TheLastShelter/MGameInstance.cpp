// Copyright TheLastShelter. All Rights Reserved.

#include "MGameInstance.h"
#include "MDataManager.h"
#include "MInventoryManager.h"
#include "MShelterInventoryManager.h"
#include "MShelterValueManager.h"
#include "MWorldTimeManager.h"
#include "MHardshipManager.h"
#include "MSpawnManager.h"

void UMGameInstance::Init()
{
	Super::Init();

	// GameInstanceSubsystem은 엔진이 이미 Initialize()를 완료한 상태
	DataManager              = GetSubsystem<UMDataManager>();
	InventoryManager         = GetSubsystem<UMInventoryManager>();
	ShelterInventoryManager  = GetSubsystem<UMShelterInventoryManager>();
	ShelterValueManager      = GetSubsystem<UMShelterValueManager>();
	WorldTimeManager         = GetSubsystem<UMWorldTimeManager>();
	HardshipManager          = GetSubsystem<UMHardshipManager>();
	SpawnManager             = GetSubsystem<UMSpawnManager>();

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Init — Data=%s  Inventory=%s  ShelterInv=%s  ShelterVal=%s  WorldTime=%s  Hardship=%s  Spawn=%s"),
		DataManager             ? TEXT("OK") : TEXT("MISSING"),
		InventoryManager        ? TEXT("OK") : TEXT("MISSING"),
		ShelterInventoryManager ? TEXT("OK") : TEXT("MISSING"),
		ShelterValueManager     ? TEXT("OK") : TEXT("MISSING"),
		WorldTimeManager        ? TEXT("OK") : TEXT("MISSING"),
		HardshipManager         ? TEXT("OK") : TEXT("MISSING"),
		SpawnManager            ? TEXT("OK") : TEXT("MISSING"));

	// FTSTicker를 이용해 매 프레임 Tick 등록
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UMGameInstance::Tick));
}

void UMGameInstance::Shutdown()
{
	// Tick 해제
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Shutdown"));
	Super::Shutdown();
}

bool UMGameInstance::Tick(float DeltaSeconds)
{
	// 1) 시간 진행
	if (WorldTimeManager)
	{
		WorldTimeManager->AdvanceTime(DeltaSeconds);
	}

	// 2) 재난 실시간 효과
	if (HardshipManager)
	{
		HardshipManager->TickHardship(DeltaSeconds);
	}

	// 3) 웨이브 타이머·스폰
	if (SpawnManager)
	{
		SpawnManager->TickSpawn(DeltaSeconds);
	}

	return true; // true = 계속 Tick
}
