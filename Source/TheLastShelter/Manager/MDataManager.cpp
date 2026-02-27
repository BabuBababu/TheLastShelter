// Copyright TheLastShelter. All Rights Reserved.

#include "MDataManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UMDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DataDirectoryPath = FPaths::ProjectDir() / TEXT("Data");
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Initialized. Data path: %s"), *DataDirectoryPath);

	// GameInstance 생성 시 모든 JSON 데이터 자동 로드
	LoadAllData();
}

void UMDataManager::Deinitialize()
{
	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Deinitialized."));
}

// ============================================================
// 전체 로드
// ============================================================

void UMDataManager::LoadAllData()
{
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loading all data from: %s"), *DataDirectoryPath);

	LoadEveData();
	LoadOrdoData();
	LoadPrimalData();
	LoadItemData();
	LoadStatData();
	LoadHiddenStatData();
	LoadSkillData();
	LoadDropItemsData();
	LoadEventData();
	LoadSpecialEventData();
	LoadEveDialogData();
	LoadOrdoDialogData();
	LoadPlayerDialogData();
	LoadLootingObjectData();

	UE_LOG(LogTemp, Log, TEXT("[MDataManager] All data loaded. Eve=%d, Ordo=%d, Item=%d"),
		EveDataArray.Num(), OrdoDataArray.Num(), ItemDataArray.Num());
}

// ============================================================
// JSON 파일 로드 헬퍼
// ============================================================

bool UMDataManager::LoadJsonFile(const FString& FileName, TArray<TSharedPtr<FJsonValue>>& OutArray) const
{
	const FString FilePath = DataDirectoryPath / FileName;

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MDataManager] Failed to load file: %s"), *FilePath);
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, OutArray))
	{
		// 단일 오브젝트일 수도 있으므로 오브젝트로 재시도
		TSharedPtr<FJsonObject> RootObj;
		TSharedRef<TJsonReader<>> Reader2 = TJsonReaderFactory<>::Create(JsonString);
		if (FJsonSerializer::Deserialize(Reader2, RootObj) && RootObj.IsValid())
		{
			// "data" 키 아래 배열이 있는 경우
			const TArray<TSharedPtr<FJsonValue>>* DataArray;
			if (RootObj->TryGetArrayField(TEXT("data"), DataArray))
			{
				OutArray = *DataArray;
				return true;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("[MDataManager] Failed to parse JSON: %s"), *FilePath);
		return false;
	}

	return true;
}

// ============================================================
// 파싱 헬퍼
// ============================================================

FMPhysicalStat UMDataManager::ParsePhysicalStat(const TSharedPtr<FJsonObject>& JsonObj)
{
	FMPhysicalStat Stat;
	if (!JsonObj.IsValid()) return Stat;

	Stat.Attack		= JsonObj->GetNumberField(TEXT("Attack"));
	Stat.Defense	= JsonObj->GetNumberField(TEXT("Defense"));
	Stat.Health		= JsonObj->GetNumberField(TEXT("Health"));
	Stat.Stamina	= JsonObj->GetNumberField(TEXT("Stamina"));
	Stat.MoveSpeed	= JsonObj->GetNumberField(TEXT("MoveSpeed"));
	Stat.WorkSpeed	= JsonObj->GetNumberField(TEXT("WorkSpeed"));
	Stat.AttackSpeed= JsonObj->GetNumberField(TEXT("AttackSpeed"));
	Stat.Recovery	= JsonObj->GetNumberField(TEXT("Recovery"));
	Stat.Accuracy	= JsonObj->GetNumberField(TEXT("Accuracy"));
	Stat.Evasion	= JsonObj->GetNumberField(TEXT("Evasion"));
	Stat.CritChance	= JsonObj->GetNumberField(TEXT("CritChance"));
	Stat.CritDamage	= JsonObj->GetNumberField(TEXT("CritDamage"));

	return Stat;
}

FMMentalStat UMDataManager::ParseMentalStat(const TSharedPtr<FJsonObject>& JsonObj)
{
	FMMentalStat Stat;
	if (!JsonObj.IsValid()) return Stat;

	Stat.Combat			= JsonObj->GetNumberField(TEXT("Combat"));
	Stat.Construction	= JsonObj->GetNumberField(TEXT("Construction"));
	Stat.Mining			= JsonObj->GetNumberField(TEXT("Mining"));
	Stat.Cooking		= JsonObj->GetNumberField(TEXT("Cooking"));
	Stat.Gardening		= JsonObj->GetNumberField(TEXT("Gardening"));
	Stat.Crafting		= JsonObj->GetNumberField(TEXT("Crafting"));
	Stat.Medicine		= JsonObj->GetNumberField(TEXT("Medicine"));
	Stat.Social			= JsonObj->GetNumberField(TEXT("Social"));
	Stat.Research		= JsonObj->GetNumberField(TEXT("Research"));
	Stat.Lewdness		= JsonObj->GetNumberField(TEXT("Lewdness"));
	Stat.Composure		= JsonObj->GetNumberField(TEXT("Composure"));
	Stat.Generosity		= JsonObj->GetNumberField(TEXT("Generosity"));

	return Stat;
}

// ============================================================
// 개별 테이블 로드
// ============================================================

void UMDataManager::LoadEveData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Eve.json"), JsonArray)) return;

	EveDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMEveData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.Affection = Obj->GetNumberField(TEXT("Affection"));
		Data.SpritePath = Obj->GetStringField(TEXT("SpritePath"));

		// PhysicalStat
		const TSharedPtr<FJsonObject>* PhysObj;
		if (Obj->TryGetObjectField(TEXT("PhysicalStat"), PhysObj))
			Data.PhysicalStat = ParsePhysicalStat(*PhysObj);

		// MentalStat
		const TSharedPtr<FJsonObject>* MentObj;
		if (Obj->TryGetObjectField(TEXT("MentalStat"), MentObj))
			Data.MentalStat = ParseMentalStat(*MentObj);

		// HiddenStats
		const TArray<TSharedPtr<FJsonValue>>* HiddenArr;
		if (Obj->TryGetArrayField(TEXT("HiddenStats"), HiddenArr))
		{
			for (const auto& H : *HiddenArr)
			{
				Data.HiddenStats.Add(static_cast<EMHiddenStatType>(static_cast<int32>(H->AsNumber())));
			}
		}

		// SkillIDs
		const TArray<TSharedPtr<FJsonValue>>* SkillArr;
		if (Obj->TryGetArrayField(TEXT("SkillIDs"), SkillArr))
		{
			for (const auto& S : *SkillArr)
				Data.SkillIDs.Add(S->AsString());
		}

		EveDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Eve entries"), EveDataArray.Num());
}

void UMDataManager::LoadOrdoData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Ordo.json"), JsonArray)) return;

	OrdoDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMOrdoData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.OrdoType = static_cast<EMOrdoType>(static_cast<int32>(Obj->GetNumberField(TEXT("OrdoType"))));
		Data.DropTableID = Obj->GetStringField(TEXT("DropTableID"));
		Data.SpritePath = Obj->GetStringField(TEXT("SpritePath"));

		const TSharedPtr<FJsonObject>* PhysObj;
		if (Obj->TryGetObjectField(TEXT("PhysicalStat"), PhysObj))
			Data.PhysicalStat = ParsePhysicalStat(*PhysObj);

		const TArray<TSharedPtr<FJsonValue>>* SkillArr;
		if (Obj->TryGetArrayField(TEXT("SkillIDs"), SkillArr))
			for (const auto& S : *SkillArr)
				Data.SkillIDs.Add(S->AsString());

		OrdoDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Ordo entries"), OrdoDataArray.Num());
}

void UMDataManager::LoadPrimalData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Primal.json"), JsonArray)) return;

	PrimalDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMPrimalData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.PrimalType = static_cast<EMPrimalType>(static_cast<int32>(Obj->GetNumberField(TEXT("PrimalType"))));
		Data.DropTableID = Obj->GetStringField(TEXT("DropTableID"));
		Data.SpritePath = Obj->GetStringField(TEXT("SpritePath"));

		PrimalDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Primal entries"), PrimalDataArray.Num());
}

void UMDataManager::LoadItemData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Item.json"), JsonArray)) return;

	ItemDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMItemData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.Description = Obj->GetStringField(TEXT("Description"));
		Data.ItemType = static_cast<EMItemType>(static_cast<int32>(Obj->GetNumberField(TEXT("ItemType"))));
		Data.EquipSlot = static_cast<EMEquipmentSlot>(static_cast<int32>(Obj->GetNumberField(TEXT("EquipSlot"))));
		Data.MaxStack = static_cast<int32>(Obj->GetNumberField(TEXT("MaxStack")));
		Data.Value = Obj->GetNumberField(TEXT("Value"));
		Data.ExposureScore = Obj->GetNumberField(TEXT("ExposureScore"));
		Data.SpritePath = Obj->GetStringField(TEXT("SpritePath"));

		// Effects
		const TSharedPtr<FJsonObject>* EffectsObj;
		if (Obj->TryGetObjectField(TEXT("Effects"), EffectsObj))
		{
			for (const auto& Pair : (*EffectsObj)->Values)
			{
				Data.Effects.Add(Pair.Key, Pair.Value->AsNumber());
			}
		}

		// BonusStat
		const TSharedPtr<FJsonObject>* BonusObj;
		if (Obj->TryGetObjectField(TEXT("BonusStat"), BonusObj))
			Data.BonusStat = ParsePhysicalStat(*BonusObj);

		ItemDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Item entries"), ItemDataArray.Num());
}

void UMDataManager::LoadStatData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Stat.json"), JsonArray)) return;

	StatDefArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMStatDefinition Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.Description = Obj->GetStringField(TEXT("Description"));
		Data.IsPhysical = Obj->GetBoolField(TEXT("IsPhysical"));

		StatDefArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Stat definitions"), StatDefArray.Num());
}

void UMDataManager::LoadHiddenStatData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("HiddenStat.json"), JsonArray)) return;

	HiddenStatDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMHiddenStatData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Type = static_cast<EMHiddenStatType>(static_cast<int32>(Obj->GetNumberField(TEXT("Type"))));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.Description = Obj->GetStringField(TEXT("Description"));

		const TSharedPtr<FJsonObject>* ModObj;
		if (Obj->TryGetObjectField(TEXT("StatModifiers"), ModObj))
		{
			for (const auto& Pair : (*ModObj)->Values)
				Data.StatModifiers.Add(Pair.Key, Pair.Value->AsNumber());
		}

		HiddenStatDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d HiddenStat entries"), HiddenStatDataArray.Num());
}

void UMDataManager::LoadSkillData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Skill.json"), JsonArray)) return;

	SkillDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMSkillData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.Description = Obj->GetStringField(TEXT("Description"));
		Data.Cooldown = Obj->GetNumberField(TEXT("Cooldown"));
		Data.Damage = Obj->GetNumberField(TEXT("Damage"));
		Data.AnimationID = Obj->GetStringField(TEXT("AnimationID"));

		SkillDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Skill entries"), SkillDataArray.Num());
}

void UMDataManager::LoadDropItemsData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("DropItems.json"), JsonArray)) return;

	DropTableDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMDropTableData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));

		const TArray<TSharedPtr<FJsonValue>>* EntriesArr;
		if (Obj->TryGetArrayField(TEXT("Entries"), EntriesArr))
		{
			for (const auto& E : *EntriesArr)
			{
				const TSharedPtr<FJsonObject>& EntryObj = E->AsObject();
				FMDropItemEntry Entry;
				Entry.ItemID = EntryObj->GetStringField(TEXT("ItemID"));
				Entry.DropRate = EntryObj->GetNumberField(TEXT("DropRate"));
				Entry.MinCount = static_cast<int32>(EntryObj->GetNumberField(TEXT("MinCount")));
				Entry.MaxCount = static_cast<int32>(EntryObj->GetNumberField(TEXT("MaxCount")));
				Data.Entries.Add(MoveTemp(Entry));
			}
		}

		DropTableDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d DropTable entries"), DropTableDataArray.Num());
}

void UMDataManager::LoadEventData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("Event.json"), JsonArray)) return;

	EventDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMEventData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.EventType = static_cast<EMEventType>(static_cast<int32>(Obj->GetNumberField(TEXT("EventType"))));
		Data.Description = Obj->GetStringField(TEXT("Description"));
		Data.TriggerProbability = Obj->GetNumberField(TEXT("TriggerProbability"));

		EventDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d Event entries"), EventDataArray.Num());
}

void UMDataManager::LoadSpecialEventData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("SpecialEvent.json"), JsonArray)) return;

	SpecialEventDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMSpecialEventData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Title = Obj->GetStringField(TEXT("Title"));

		const TArray<TSharedPtr<FJsonValue>>* LinesArr;
		if (Obj->TryGetArrayField(TEXT("ScriptLines"), LinesArr))
		{
			for (const auto& L : *LinesArr)
				Data.ScriptLines.Add(L->AsString());
		}

		SpecialEventDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d SpecialEvent entries"), SpecialEventDataArray.Num());
}

void UMDataManager::LoadEveDialogData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("EveDialog.json"), JsonArray)) return;

	EveDialogDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMEveDialogData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.EveID = Obj->GetStringField(TEXT("EveID"));
		Data.Emotion = static_cast<EMEmotion>(static_cast<int32>(Obj->GetNumberField(TEXT("Emotion"))));
		Data.EventCondition = static_cast<EMEventType>(static_cast<int32>(Obj->GetNumberField(TEXT("EventCondition"))));
		Data.DialogText = Obj->GetStringField(TEXT("DialogText"));

		EveDialogDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d EveDialog entries"), EveDialogDataArray.Num());
}

void UMDataManager::LoadOrdoDialogData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("OrdoDialog.json"), JsonArray)) return;

	OrdoDialogDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMOrdoDialogData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.OrdoID = Obj->GetStringField(TEXT("OrdoID"));
		Data.DialogText = Obj->GetStringField(TEXT("DialogText"));

		OrdoDialogDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d OrdoDialog entries"), OrdoDialogDataArray.Num());
}

void UMDataManager::LoadPlayerDialogData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("PlayerDialog.json"), JsonArray)) return;

	PlayerDialogDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMPlayerDialogData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.EventCondition = static_cast<EMEventType>(static_cast<int32>(Obj->GetNumberField(TEXT("EventCondition"))));
		Data.DialogText = Obj->GetStringField(TEXT("DialogText"));

		PlayerDialogDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d PlayerDialog entries"), PlayerDialogDataArray.Num());
}

void UMDataManager::LoadLootingObjectData()
{
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	if (!LoadJsonFile(TEXT("LootingObject.json"), JsonArray)) return;

	LootingObjectDataArray.Empty();
	for (const auto& Val : JsonArray)
	{
		const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
		if (!Obj.IsValid()) continue;

		FMLootingObjectData Data;
		Data.ID = Obj->GetStringField(TEXT("ID"));
		Data.Name = Obj->GetStringField(TEXT("Name"));
		Data.SpritePath = Obj->GetStringField(TEXT("SpritePath"));

		const TArray<TSharedPtr<FJsonValue>>* ItemArr;
		if (Obj->TryGetArrayField(TEXT("SpawnItemIDs"), ItemArr))
		{
			for (const auto& I : *ItemArr)
				Data.SpawnItemIDs.Add(I->AsString());
		}

		LootingObjectDataArray.Add(MoveTemp(Data));
	}
	UE_LOG(LogTemp, Log, TEXT("[MDataManager] Loaded %d LootingObject entries"), LootingObjectDataArray.Num());
}

// ============================================================
// ID로 조회
// ============================================================

bool UMDataManager::GetEveDataByID(const FString& ID, FMEveData& OutData) const
{
	for (const auto& Data : EveDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

bool UMDataManager::GetOrdoDataByID(const FString& ID, FMOrdoData& OutData) const
{
	for (const auto& Data : OrdoDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

bool UMDataManager::GetPrimalDataByID(const FString& ID, FMPrimalData& OutData) const
{
	for (const auto& Data : PrimalDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

bool UMDataManager::GetItemDataByID(const FString& ID, FMItemData& OutData) const
{
	for (const auto& Data : ItemDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

bool UMDataManager::GetSkillDataByID(const FString& ID, FMSkillData& OutData) const
{
	for (const auto& Data : SkillDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

bool UMDataManager::GetDropTableByID(const FString& ID, FMDropTableData& OutData) const
{
	for (const auto& Data : DropTableDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

bool UMDataManager::GetLootingObjectDataByID(const FString& ID, FMLootingObjectData& OutData) const
{
	for (const auto& Data : LootingObjectDataArray)
	{
		if (Data.ID == ID) { OutData = Data; return true; }
	}
	return false;
}

// ============================================================
// 다이얼로그 필터 조회
// ============================================================

TArray<FMEveDialogData> UMDataManager::GetEveDialogs(const FString& EveID, EMEmotion Emotion, EMEventType EventType) const
{
	TArray<FMEveDialogData> Results;
	for (const auto& Data : EveDialogDataArray)
	{
		if (Data.EveID == EveID && Data.Emotion == Emotion && Data.EventCondition == EventType)
			Results.Add(Data);
	}
	return Results;
}

TArray<FMOrdoDialogData> UMDataManager::GetOrdoDialogs(const FString& OrdoID) const
{
	TArray<FMOrdoDialogData> Results;
	for (const auto& Data : OrdoDialogDataArray)
	{
		if (Data.OrdoID == OrdoID)
			Results.Add(Data);
	}
	return Results;
}

TArray<FMPlayerDialogData> UMDataManager::GetPlayerDialogs(EMEventType EventType) const
{
	TArray<FMPlayerDialogData> Results;
	for (const auto& Data : PlayerDialogDataArray)
	{
		if (Data.EventCondition == EventType)
			Results.Add(Data);
	}
	return Results;
}
