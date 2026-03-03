// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MGameInstance.generated.h"

class UMDataManager;
class UMInventoryManager;
class UMShelterInventoryManager;
class UMShelterValueManager;
class UMWorldTimeManager;
class UMHardshipManager;
class UMSpawnManager;

/**
 * MGameInstance
 *
 * 프로젝트 전역 GameInstance.
 * 모든 커스텀 GameInstanceSubsystem을 캐싱하여 빠른 접근을 제공하고,
 * Tick이 필요한 시스템(시간·재난·스폰)은 매 프레임 구동합니다.
 *
 * DefaultEngine.ini에서 GameInstanceClass 지정 필요:
 *   [/Script/EngineSettings.GameMapsSettings]
 *   GameInstanceClass=/Script/TheLastShelter.MGameInstance
 */
UCLASS()
class THELASTSHELTER_API UMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	// ============================================================
	// 서브시스템 캐시 (빠른 접근)
	// ============================================================

	/** 데이터 매니저 (xlsx→json 로드) */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMDataManager* GetDataManager() const { return DataManager; }

	/** 플레이어 인벤토리 */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMInventoryManager* GetInventoryManager() const { return InventoryManager; }

	/** 쉘터 창고 인벤토리 */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMShelterInventoryManager* GetShelterInventoryManager() const { return ShelterInventoryManager; }

	/** 기지 가치 계산 */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMShelterValueManager* GetShelterValueManager() const { return ShelterValueManager; }

	/** 인게임 시간 */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMWorldTimeManager* GetWorldTimeManager() const { return WorldTimeManager; }

	/** 재난/역경 시스템 */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMHardshipManager* GetHardshipManager() const { return HardshipManager; }

	/** 스폰 시스템 */
	UFUNCTION(BlueprintPure, Category = "Game|Manager")
	UMSpawnManager* GetSpawnManager() const { return SpawnManager; }

private:
	/** FTSTicker 기반 Tick 핸들 */
	FTSTicker::FDelegateHandle TickHandle;

	/** Tick 콜백 */
	bool Tick(float DeltaSeconds);

	// ============================================================
	// 캐싱된 서브시스템 포인터
	// ============================================================

	UPROPERTY()
	TObjectPtr<UMDataManager> DataManager;

	UPROPERTY()
	TObjectPtr<UMInventoryManager> InventoryManager;

	UPROPERTY()
	TObjectPtr<UMShelterInventoryManager> ShelterInventoryManager;

	UPROPERTY()
	TObjectPtr<UMShelterValueManager> ShelterValueManager;

	UPROPERTY()
	TObjectPtr<UMWorldTimeManager> WorldTimeManager;

	UPROPERTY()
	TObjectPtr<UMHardshipManager> HardshipManager;

	UPROPERTY()
	TObjectPtr<UMSpawnManager> SpawnManager;
};
