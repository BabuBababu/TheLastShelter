// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MDataManager.generated.h"

/**
 * MDataManager
 * JSON 파일에서 게임 데이터를 로딩하여 메모리에 보관하고,
 * 각 시스템이 ID로 데이터를 조회할 수 있도록 제공합니다.
 *
 * JSON 파일 경로: {ProjectDir}/Data/{TableName}.json
 */
UCLASS()
class THELASTSHELTER_API UMDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// --- Subsystem Lifecycle ---
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- 전체 데이터 로드 ---
	UFUNCTION(BlueprintCallable, Category = "Data")
	void LoadAllData();

	// --- Eve ---
	UFUNCTION(BlueprintCallable, Category = "Data|Eve")
	const TArray<FMEveData>& GetAllEveData() const { return EveDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Eve")
	bool GetEveDataByID(int32 Id, FMEveData& OutData) const;

	// --- Ordo ---
	UFUNCTION(BlueprintCallable, Category = "Data|Ordo")
	const TArray<FMOrdoData>& GetAllOrdoData() const { return OrdoDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Ordo")
	bool GetOrdoDataByID(int32 Id, FMOrdoData& OutData) const;

	// --- Primal ---
	UFUNCTION(BlueprintCallable, Category = "Data|Primal")
	const TArray<FMPrimalData>& GetAllPrimalData() const { return PrimalDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Primal")
	bool GetPrimalDataByID(int32 Id, FMPrimalData& OutData) const;

	// --- Item ---
	UFUNCTION(BlueprintCallable, Category = "Data|Item")
	const TArray<FMItemData>& GetAllItemData() const { return ItemDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Item")
	bool GetItemDataByID(int32 Id, FMItemData& OutData) const;

	// --- Stat ---
	UFUNCTION(BlueprintCallable, Category = "Data|Stat")
	const TArray<FMStatDefinition>& GetAllStatDefinitions() const { return StatDefArray; }

	// --- HiddenStat ---
	UFUNCTION(BlueprintCallable, Category = "Data|HiddenStat")
	const TArray<FMHiddenStatData>& GetAllHiddenStatData() const { return HiddenStatDataArray; }

	// --- Skill ---
	UFUNCTION(BlueprintCallable, Category = "Data|Skill")
	const TArray<FMSkillData>& GetAllSkillData() const { return SkillDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Skill")
	bool GetSkillDataByID(int32 Id, FMSkillData& OutData) const;

	// --- DropTable ---
	UFUNCTION(BlueprintCallable, Category = "Data|Drop")
	bool GetDropTableByID(int32 Id, FMDropTableData& OutData) const;

	// --- Event ---
	UFUNCTION(BlueprintCallable, Category = "Data|Event")
	const TArray<FMEventData>& GetAllEventData() const { return EventDataArray; }

	// --- SpecialEvent ---
	UFUNCTION(BlueprintCallable, Category = "Data|Event")
	const TArray<FMSpecialEventData>& GetAllSpecialEventData() const { return SpecialEventDataArray; }

	// --- Dialog ---
	UFUNCTION(BlueprintCallable, Category = "Data|Dialog")
	TArray<FMEveDialogData> GetEveDialogs(int32 EveId, EMEmotion Emotion, EMEventType EventType) const;

	UFUNCTION(BlueprintCallable, Category = "Data|Dialog")
	TArray<FMOrdoDialogData> GetOrdoDialogs(int32 OrdoId) const;

	UFUNCTION(BlueprintCallable, Category = "Data|Dialog")
	TArray<FMPlayerDialogData> GetPlayerDialogs(EMEventType EventType) const;

	// --- LootingObject ---
	UFUNCTION(BlueprintCallable, Category = "Data|Loot")
	bool GetLootingObjectDataByID(int32 Id, FMLootingObjectData& OutData) const;

private:
	// --- 개별 테이블 로드 ---
	bool LoadJsonFile(const FString& FileName, TArray<TSharedPtr<FJsonValue>>& OutArray) const;

	void LoadEveData();
	void LoadOrdoData();
	void LoadPrimalData();
	void LoadItemData();
	void LoadStatData();
	void LoadHiddenStatData();
	void LoadSkillData();
	void LoadDropItemsData();
	void LoadEventData();
	void LoadSpecialEventData();
	void LoadEveDialogData();
	void LoadOrdoDialogData();
	void LoadPlayerDialogData();
	void LoadLootingObjectData();

	// --- 파싱 헬퍼 ---
	static FMPhysicalStat ParsePhysicalStat(const TSharedPtr<FJsonObject>& JsonObj);
	static FMMentalStat ParseMentalStat(const TSharedPtr<FJsonObject>& JsonObj);

	// --- 데이터 저장소 (Array) ---
	UPROPERTY()
	TArray<FMEveData> EveDataArray;

	UPROPERTY()
	TArray<FMOrdoData> OrdoDataArray;

	UPROPERTY()
	TArray<FMPrimalData> PrimalDataArray;

	UPROPERTY()
	TArray<FMItemData> ItemDataArray;

	UPROPERTY()
	TArray<FMStatDefinition> StatDefArray;

	UPROPERTY()
	TArray<FMHiddenStatData> HiddenStatDataArray;

	UPROPERTY()
	TArray<FMSkillData> SkillDataArray;

	UPROPERTY()
	TArray<FMDropTableData> DropTableDataArray;

	UPROPERTY()
	TArray<FMEventData> EventDataArray;

	UPROPERTY()
	TArray<FMSpecialEventData> SpecialEventDataArray;

	UPROPERTY()
	TArray<FMEveDialogData> EveDialogDataArray;

	UPROPERTY()
	TArray<FMOrdoDialogData> OrdoDialogDataArray;

	UPROPERTY()
	TArray<FMPlayerDialogData> PlayerDialogDataArray;

	UPROPERTY()
	TArray<FMLootingObjectData> LootingObjectDataArray;

	// --- TMap 캐시 (Id → 구조체, O(1) 조회) ---
	TMap<int32, FMEveData> EveDataMap;
	TMap<int32, FMOrdoData> OrdoDataMap;
	TMap<int32, FMPrimalData> PrimalDataMap;
	TMap<int32, FMItemData> ItemDataMap;
	TMap<int32, FMSkillData> SkillDataMap;
	TMap<int32, FMDropTableData> DropTableDataMap;
	TMap<int32, FMLootingObjectData> LootingObjectDataMap;

	/** JSON 데이터 경로 */
	FString DataDirectoryPath;
};
