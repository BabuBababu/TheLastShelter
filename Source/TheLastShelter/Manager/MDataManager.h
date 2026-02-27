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
	bool GetEveDataByID(const FString& ID, FMEveData& OutData) const;

	// --- Ordo ---
	UFUNCTION(BlueprintCallable, Category = "Data|Ordo")
	const TArray<FMOrdoData>& GetAllOrdoData() const { return OrdoDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Ordo")
	bool GetOrdoDataByID(const FString& ID, FMOrdoData& OutData) const;

	// --- Primal ---
	UFUNCTION(BlueprintCallable, Category = "Data|Primal")
	const TArray<FMPrimalData>& GetAllPrimalData() const { return PrimalDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Primal")
	bool GetPrimalDataByID(const FString& ID, FMPrimalData& OutData) const;

	// --- Item ---
	UFUNCTION(BlueprintCallable, Category = "Data|Item")
	const TArray<FMItemData>& GetAllItemData() const { return ItemDataArray; }

	UFUNCTION(BlueprintCallable, Category = "Data|Item")
	bool GetItemDataByID(const FString& ID, FMItemData& OutData) const;

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
	bool GetSkillDataByID(const FString& ID, FMSkillData& OutData) const;

	// --- DropTable ---
	UFUNCTION(BlueprintCallable, Category = "Data|Drop")
	bool GetDropTableByID(const FString& ID, FMDropTableData& OutData) const;

	// --- Event ---
	UFUNCTION(BlueprintCallable, Category = "Data|Event")
	const TArray<FMEventData>& GetAllEventData() const { return EventDataArray; }

	// --- SpecialEvent ---
	UFUNCTION(BlueprintCallable, Category = "Data|Event")
	const TArray<FMSpecialEventData>& GetAllSpecialEventData() const { return SpecialEventDataArray; }

	// --- Dialog ---
	UFUNCTION(BlueprintCallable, Category = "Data|Dialog")
	TArray<FMEveDialogData> GetEveDialogs(const FString& EveID, EMEmotion Emotion, EMEventType EventType) const;

	UFUNCTION(BlueprintCallable, Category = "Data|Dialog")
	TArray<FMOrdoDialogData> GetOrdoDialogs(const FString& OrdoID) const;

	UFUNCTION(BlueprintCallable, Category = "Data|Dialog")
	TArray<FMPlayerDialogData> GetPlayerDialogs(EMEventType EventType) const;

	// --- LootingObject ---
	UFUNCTION(BlueprintCallable, Category = "Data|Loot")
	bool GetLootingObjectDataByID(const FString& ID, FMLootingObjectData& OutData) const;

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

	// --- 데이터 저장소 ---
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

	/** JSON 데이터 경로 */
	FString DataDirectoryPath;
};
