// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MDataTypes.generated.h"

// ============================================================
// Enums
// ============================================================

/** 감정 상태 */
UENUM(BlueprintType)
enum class EMEmotion : uint8
{
	Normal		UMETA(DisplayName = "보통"),
	Joyful		UMETA(DisplayName = "즐거움"),
	Happy		UMETA(DisplayName = "기쁨"),
	Sad			UMETA(DisplayName = "슬픔"),
	Depressed	UMETA(DisplayName = "우울"),
	Sick		UMETA(DisplayName = "병듦"),
	Shy			UMETA(DisplayName = "부끄러움"),
	Worried		UMETA(DisplayName = "고민"),
	Aroused		UMETA(DisplayName = "발정")
};

/** 아이템 타입 */
UENUM(BlueprintType)
enum class EMItemType : uint8
{
	None		UMETA(DisplayName = "없음"),
	Survival	UMETA(DisplayName = "생존"),
	Consumable	UMETA(DisplayName = "소비"),
	Tool		UMETA(DisplayName = "도구"),
	Equipment	UMETA(DisplayName = "장비"),
	Building	UMETA(DisplayName = "건축"),
	Misc		UMETA(DisplayName = "기타")
};

/** 장비 슬롯 */
UENUM(BlueprintType)
enum class EMEquipmentSlot : uint8
{
	None		UMETA(DisplayName = "없음"),
	Weapon		UMETA(DisplayName = "무기"),
	WeaponSub1	UMETA(DisplayName = "무기 보조1"),
	WeaponSub2	UMETA(DisplayName = "무기 보조2"),
	WeaponSub3	UMETA(DisplayName = "무기 보조3"),
	Head		UMETA(DisplayName = "머리"),
	Chest		UMETA(DisplayName = "가슴"),
	Legs		UMETA(DisplayName = "다리"),
	Feet		UMETA(DisplayName = "발"),
	Gloves		UMETA(DisplayName = "장갑"),
	Waist		UMETA(DisplayName = "허리")
};

/** 이벤트 타입 */
UENUM(BlueprintType)
enum class EMEventType : uint8
{
	RestVisit			UMETA(DisplayName = "쉴 때 방문"),
	WorkVisit			UMETA(DisplayName = "작업 중 방문"),
	InCombat			UMETA(DisplayName = "전투중"),
	SickVisit			UMETA(DisplayName = "병문안"),
	SleepTogether		UMETA(DisplayName = "함께 잠자기"),
	ReceiveGift			UMETA(DisplayName = "선물 받기"),
	GiveGift			UMETA(DisplayName = "선물 주기"),
	Bump				UMETA(DisplayName = "부딪힘"),
	Scold				UMETA(DisplayName = "잘못 따짐"),
	Flirt				UMETA(DisplayName = "플러팅"),
	Confess				UMETA(DisplayName = "고백"),
	ReceiveConfession	UMETA(DisplayName = "고백 받음")
};

/** 오르도 타입 */
UENUM(BlueprintType)
enum class EMOrdoType : uint8
{
	Axiom	UMETA(DisplayName = "엑시움"),
	Synthea	UMETA(DisplayName = "신테아")
};

/** 프라이멀 타입 */
UENUM(BlueprintType)
enum class EMPrimalType : uint8
{
	Animal	UMETA(DisplayName = "동물"),
	Plant	UMETA(DisplayName = "식물")
};

// ============================================================
// Animation State Enums
// ============================================================

/** Eve 애니메이션 상태 */
UENUM(BlueprintType)
enum class EMEveAnimState : uint8
{
	// ---- 기본 이동 (무장 해제) ----
	Walk_Down			UMETA(DisplayName = "아래 걷기"),
	Walk_Up				UMETA(DisplayName = "위 걷기"),
	Walk_Left			UMETA(DisplayName = "왼쪽 걷기"),
	Walk_Right			UMETA(DisplayName = "오른쪽 걷기"),

	// ---- 총 이동 ----
	GunWalk_Down		UMETA(DisplayName = "총 아래 걷기"),
	GunWalk_Up			UMETA(DisplayName = "총 위 걷기"),
	GunWalk_Left		UMETA(DisplayName = "총 왼쪽 걷기"),
	GunWalk_Right		UMETA(DisplayName = "총 오른쪽 걷기"),

	// ---- 기본 뛰기 ----
	Run_Down			UMETA(DisplayName = "아래 뛰기"),
	Run_Up				UMETA(DisplayName = "위 뛰기"),
	Run_Left			UMETA(DisplayName = "왼쪽 뛰기"),
	Run_Right			UMETA(DisplayName = "오른쪽 뛰기"),

	// ---- 총 뛰기 ----
	GunRun_Down			UMETA(DisplayName = "총 아래 뛰기"),
	GunRun_Up			UMETA(DisplayName = "총 위 뛰기"),
	GunRun_Left			UMETA(DisplayName = "총 왼쪽 뛰기"),
	GunRun_Right		UMETA(DisplayName = "총 오른쪽 뛰기"),

	// ---- 대기 ----
	Idle				UMETA(DisplayName = "대기하기"),
	GunIdle				UMETA(DisplayName = "총 대기하기"),

	// ---- 전투 ----
	GunShot_Down		UMETA(DisplayName = "총 아래 사격"),
	GunShot_Up			UMETA(DisplayName = "총 위 사격"),
	GunShot_Left		UMETA(DisplayName = "총 왼쪽 사격"),
	GunShot_Right		UMETA(DisplayName = "총 오른쪽 사격"),
	Hit					UMETA(DisplayName = "피격"),

	// ---- 쓰러짐 ----
	Down				UMETA(DisplayName = "쓰러짐"),
	GunDown				UMETA(DisplayName = "총 쓰러짐"),

	// ---- 작업 ----
	Gather				UMETA(DisplayName = "수집하기"),
	ChopTree			UMETA(DisplayName = "나무캐기"),
	Craft				UMETA(DisplayName = "제작하기"),
	Pickaxe				UMETA(DisplayName = "곡괭이질"),
	SitRest				UMETA(DisplayName = "앉아서 휴식"),

	// ---- 상자 운반 ----
	BoxCarry_Down		UMETA(DisplayName = "상자 아래 걷기"),
	BoxCarry_Up			UMETA(DisplayName = "상자 위 걷기"),
	BoxCarry_Left		UMETA(DisplayName = "상자 왼쪽 걷기"),
	BoxCarry_Right		UMETA(DisplayName = "상자 오른쪽 걷기"),

	// ---- 성인 ----
	Sex_Missionary		UMETA(DisplayName = "정상위"),
	Sex_Doggy			UMETA(DisplayName = "후배위"),
	Sex_Cowgirl			UMETA(DisplayName = "카우걸"),
	Sex_Masturbation	UMETA(DisplayName = "자위"),

	MAX					UMETA(Hidden)
};

/** Ordo 애니메이션 상태 */
UENUM(BlueprintType)
enum class EMOrdoAnimState : uint8
{
	// ---- 총 이동 ----
	GunWalk_Down		UMETA(DisplayName = "총 아래 걷기"),
	GunWalk_Up			UMETA(DisplayName = "총 위 걷기"),
	GunWalk_Left		UMETA(DisplayName = "총 왼쪽 걷기"),
	GunWalk_Right		UMETA(DisplayName = "총 오른쪽 걷기"),

	// ---- 총 뛰기 ----
	GunRun_Down			UMETA(DisplayName = "총 아래 뛰기"),
	GunRun_Up			UMETA(DisplayName = "총 위 뛰기"),
	GunRun_Left			UMETA(DisplayName = "총 왼쪽 뛰기"),
	GunRun_Right		UMETA(DisplayName = "총 오른쪽 뛰기"),

	// ---- 대기 / 쓰러짐 ----
	GunIdle				UMETA(DisplayName = "총 대기하기"),

	// ---- 전투 ----
	GunShot_Down		UMETA(DisplayName = "총 아래 사격"),
	GunShot_Up			UMETA(DisplayName = "총 위 사격"),
	GunShot_Left		UMETA(DisplayName = "총 왼쪽 사격"),
	GunShot_Right		UMETA(DisplayName = "총 오른쪽 사격"),
	Hit					UMETA(DisplayName = "피격"),

	Down				UMETA(DisplayName = "쓰러짐"),

	MAX					UMETA(Hidden)
};

/** 히든 스탯 타입 */
UENUM(BlueprintType)
enum class EMHiddenStatType : uint8
{
	None			UMETA(DisplayName = "없음"),
	StrongWill		UMETA(DisplayName = "강한의지"),
	Toughness		UMETA(DisplayName = "강인함"),
	Swiftness		UMETA(DisplayName = "신속함"),
	Exhibitionist	UMETA(DisplayName = "노출광"),
	Nymphomaniac	UMETA(DisplayName = "색정광"),
	Nocturnal		UMETA(DisplayName = "야행성"),
	Gentle			UMETA(DisplayName = "상냥함"),
	Sharpshooter	UMETA(DisplayName = "백발백중"),
	Calm			UMETA(DisplayName = "침착함"),
	Masochist		UMETA(DisplayName = "피학적"),
	Immune			UMETA(DisplayName = "면역 체질"),
	Nudist			UMETA(DisplayName = "나체주의"),
	Diligent		UMETA(DisplayName = "근면 성실"),
	Beautiful		UMETA(DisplayName = "아름다움"),
	Agile			UMETA(DisplayName = "재빠름")
};

// ============================================================
// Game Data Row Base
// ============================================================

/**
 * 모든 게임 데이터 구조체의 기반.
 * xlsx에서 고유 int32 ID(Id)를 부여하며, TMap<int32, FMXxxData> 형태로 관리됩니다.
 */
USTRUCT(BlueprintType)
struct FMGameDataRowBase
{
	GENERATED_BODY()

	/** Game Data ID — xlsx 첫 번째 열의 고유 정수 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameData")
	int32 Id = 0;
};

// ============================================================
// Stat Structs
// ============================================================

/** 신체 스탯 */
USTRUCT(BlueprintType)
struct FMPhysicalStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Attack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Defense = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float WorkSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Recovery = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Accuracy = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float Evasion = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float CritChance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Physical")
	float CritDamage = 1.5f;
};

/** 정신 스탯 (Eve 전용) */
USTRUCT(BlueprintType)
struct FMMentalStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Combat = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Construction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Mining = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Cooking = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Gardening = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Crafting = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Medicine = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Social = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Research = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Lewdness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Composure = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Mental")
	float Generosity = 0.f;
};

// ============================================================
// Data Table Row Structs
// ============================================================

/** Eve 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMEveData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	FMPhysicalStat PhysicalStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	FMMentalStat MentalStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	float Affection = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	TArray<EMHiddenStatType> HiddenStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	TArray<int32> SkillIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	FString SpritePath;
};

/** Ordo 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMOrdoData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	EMOrdoType OrdoType = EMOrdoType::Axiom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FMPhysicalStat PhysicalStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	TArray<int32> SkillIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	int32 DropTableID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FString SpritePath;
};

/** Primal 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMPrimalData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	EMPrimalType PrimalType = EMPrimalType::Animal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	int32 DropTableID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	FString SpritePath;
};

/** Item 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMItemData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EMItemType ItemType = EMItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EMEquipmentSlot EquipSlot = EMEquipmentSlot::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Value = 0.f;

	/** 아이템 효과 (HP회복량, 버프수치 등 범용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TMap<FString, float> Effects;

	/** 장비 착용 시 추가 스탯 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FMPhysicalStat BonusStat;

	/** 노출도 점수 (나체주의 히든스탯용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ExposureScore = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString SpritePath;
};

/** Stat 테이블 데이터 (스탯 종류 정의) */
USTRUCT(BlueprintType)
struct FMStatDefinition : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	bool IsPhysical = true;
};

/** HiddenStat 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMHiddenStatData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenStat")
	EMHiddenStatType Type = EMHiddenStatType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenStat")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenStat")
	FString Description;

	/** 히든 스탯이 적용하는 효과 (스탯명 → 배율) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenStat")
	TMap<FString, float> StatModifiers;
};

/** Skill 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMSkillData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Cooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float Damage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FString AnimationID;
};

/** DropItems 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMDropItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 ItemId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct FMDropTableData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<FMDropItemEntry> Entries;
};

/** Event 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMEventData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EMEventType EventType = EMEventType::RestVisit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float TriggerProbability = 1.f;
};

/** SpecialEvent 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMSpecialEventData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialEvent")
	FString Title;

	/** 대사 스크립트 ('{EveName}'이 실제 이브 이름으로 치환) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialEvent")
	TArray<FString> ScriptLines;
};

/** EveDialog 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMEveDialogData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	int32 EveId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	EMEmotion Emotion = EMEmotion::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	EMEventType EventCondition = EMEventType::RestVisit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString DialogText;
};

/** OrdoDialog 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMOrdoDialogData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	int32 OrdoId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString DialogText;
};

/** PlayerDialog 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMPlayerDialogData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	EMEventType EventCondition = EMEventType::RestVisit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString DialogText;
};

/** LootingObject 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMLootingObjectData : public FMGameDataRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	FString Name;

	/** 루팅 시 스폰할 아이템 Id 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	TArray<int32> SpawnItemIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	FString SpritePath;
};

// ============================================================
// Inventory Structs
// ============================================================

/** 인벤토리 슬롯 한 칸 */
USTRUCT(BlueprintType)
struct FMInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 ItemId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Count = 0;

	bool IsEmpty() const { return ItemId <= 0 || Count <= 0; }
};

// ============================================================
// Shelter Inventory
// ============================================================

/** 쉘터 저장소 카테고리 */
UENUM(BlueprintType)
enum class EMShelterStorageCategory : uint8
{
	General		UMETA(DisplayName = "일반"),
	Resource	UMETA(DisplayName = "자원"),
	Building	UMETA(DisplayName = "건축"),
	Crafting	UMETA(DisplayName = "제작"),
	Equipment	UMETA(DisplayName = "장비"),
	Consumable	UMETA(DisplayName = "소비")
};

// ============================================================
// Ordo Spawn DataTable Row
// ============================================================

class AMOrdoCharacter;

/**
 * UE DataTable 행 구조체.
 * MSpawnManager가 웨이브별 오르도 스폰 구성을 읽을 때 사용.
 * Row Name = 고유 스폰 ID (예: "Axiom_Grunt_01")
 */
USTRUCT(BlueprintType)
struct FMOrdoSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

	/** MDataManager의 OrdoData Id (JSON 기반 스탯 초기화용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 OrdoDataId = 0;

	/** 스폰할 Ordo 블루프린트 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSoftClassPtr<AMOrdoCharacter> OrdoClass;

	/** 기지 가치 대비 이 유닛의 위협도 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float ThreatWeight = 1.f;

	/** 이 행이 활성화되는 최소 기지 가치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float MinBaseValue = 0.f;

	/** 이 행이 활성화되는 최대 기지 가치 (0 = 무제한) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float MaxBaseValue = 0.f;

	/** 스폰 확률 가중치 (높을수록 자주 선택됨) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnWeight = 1.f;
};

// ============================================================
// Wave Definition
// ============================================================

/**
 * 웨이브 정의 구조체.
 * MSpawnManager가 하루 침공 시 참조하는 웨이브 구성.
 */
USTRUCT(BlueprintType)
struct FMWaveDefinition
{
	GENERATED_BODY()

	/** 이 웨이브가 활성화되는 최소 기지 가치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float MinBaseValue = 0.f;

	/** 웨이브당 최소 스폰 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 MinSpawnCount = 1;

	/** 웨이브당 최대 스폰 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 MaxSpawnCount = 3;

	/** 기지 가치 100당 추가 스폰 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float ExtraSpawnPerBaseValue = 0.01f;

	/** 웨이브 간 딜레이 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float WaveInterval = 5.f;
};

// ============================================================
// 시간대
// ============================================================

/** 시간대 구분 */
UENUM(BlueprintType)
enum class EMTimeOfDay : uint8
{
	Dawn		UMETA(DisplayName = "새벽"),    // 0.00 ~ 0.25
	Day			UMETA(DisplayName = "낮"),      // 0.25 ~ 0.50
	Dusk		UMETA(DisplayName = "저녁"),    // 0.50 ~ 0.75
	Night		UMETA(DisplayName = "밤")       // 0.75 ~ 1.00
};

// ============================================================
// Hardship (재난) 시스템
// ============================================================

/** 재난/역경 타입 */
UENUM(BlueprintType)
enum class EMHardshipType : uint8
{
	None				UMETA(DisplayName = "없음"),
	OrdoInvasion		UMETA(DisplayName = "오르도 침공"),
	Plague				UMETA(DisplayName = "전염병"),
	Monsoon				UMETA(DisplayName = "장마"),
	Typhoon				UMETA(DisplayName = "태풍"),
	Blizzard			UMETA(DisplayName = "폭설"),
	ColdWave			UMETA(DisplayName = "한파"),
	Heatwave			UMETA(DisplayName = "폭염"),
	Drought				UMETA(DisplayName = "가뭄")
};

/** 재난 심각도 */
UENUM(BlueprintType)
enum class EMHardshipSeverity : uint8
{
	Minor		UMETA(DisplayName = "경미"),
	Moderate	UMETA(DisplayName = "보통"),
	Severe		UMETA(DisplayName = "심각"),
	Catastrophic UMETA(DisplayName = "재앙")
};

/**
 * 재난 이벤트 정의.
 * MHardshipManager가 재난 발생/진행/종료를 관리할 때 사용.
 */
USTRUCT(BlueprintType)
struct FMHardshipEventData
{
	GENERATED_BODY()

	/** 재난 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	EMHardshipType HardshipType = EMHardshipType::None;

	/** 심각도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	EMHardshipSeverity Severity = EMHardshipSeverity::Minor;

	/** 지속 시간 (인게임 일 수, 0이면 즉시 종료 = 침공 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	int32 DurationDays = 0;

	/** 남은 지속 일수 (런타임) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	int32 RemainingDays = 0;

	/** 이 재난이 활성화되는 최소 기지 가치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	float MinBaseValue = 0.f;

	/** 기본 발생 확률 (0~1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	float BaseProbability = 0.1f;

	/** 기지 가치 100당 확률 보정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	float ProbabilityPerBaseValue = 0.001f;

	/** 이 재난이 발생 가능한 시간대 (비어있으면 아무 시간대) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	TArray<EMTimeOfDay> AllowedTimeOfDay;

	/** 이 재난이 발생 가능한 최소 경과 일수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	int32 MinDay = 1;

	/** 침공의 경우 사용할 웨이브 정의 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hardship")
	TArray<FMWaveDefinition> InvasionWaves;
};

// ============================================================
// AI Task System
// ============================================================

/** 태스크 카테고리 — 전투 / 생활 구분 */
UENUM(BlueprintType)
enum class EMTaskCategory : uint8
{
	None		UMETA(DisplayName = "없음"),
	Combat		UMETA(DisplayName = "전투"),
	Life		UMETA(DisplayName = "생활")
};

/** 태스크 유형 — Eve/Ordo 모든 행동을 정형화 */
UENUM(BlueprintType)
enum class EMTaskType : uint8
{
	None					UMETA(DisplayName = "없음"),

	// ---- 공통 전투 ----
	Attack					UMETA(DisplayName = "공격"),
	ForceAttack				UMETA(DisplayName = "강제 공격"),
	HoldPosition			UMETA(DisplayName = "위치 고수"),
	ForceMove				UMETA(DisplayName = "강제 이동"),

	// ---- Eve 전투 ----
	RetreatToShelter		UMETA(DisplayName = "숙소로 퇴각"),
	UseSkill				UMETA(DisplayName = "스킬 사용"),

	// ---- Ordo 전투 ----
	Retreat					UMETA(DisplayName = "퇴각"),
	DestroyStorage			UMETA(DisplayName = "창고 부수기"),
	Kidnap					UMETA(DisplayName = "납치하기"),
	AttackDefenseTower		UMETA(DisplayName = "방어타워 공격"),

	// ---- Eve 생활 ----
	Idle					UMETA(DisplayName = "대기"),
	Move					UMETA(DisplayName = "이동"),
	Carry					UMETA(DisplayName = "운반"),
	ForceCarry				UMETA(DisplayName = "강제 운반"),
	Rest					UMETA(DisplayName = "휴식"),
	ForceRest				UMETA(DisplayName = "강제 휴식"),
	Sex						UMETA(DisplayName = "섹스"),
	ForceSex				UMETA(DisplayName = "강제 섹스"),
	Masturbation			UMETA(DisplayName = "자위"),
	ForceMasturbation		UMETA(DisplayName = "강제 자위"),

	MAX						UMETA(Hidden)
};

/** 태스크 상태 — 대기 → 진행 → 완료 */
UENUM(BlueprintType)
enum class EMTaskState : uint8
{
	Pending		UMETA(DisplayName = "대기"),
	InProgress	UMETA(DisplayName = "진행중"),
	Completed	UMETA(DisplayName = "완료"),
	Cancelled	UMETA(DisplayName = "취소됨")
};

/** 태스크 우선순위 — Force 태스크는 Interrupt 사용 */
UENUM(BlueprintType)
enum class EMTaskPriority : uint8
{
	Low			UMETA(DisplayName = "낮음"),
	Normal		UMETA(DisplayName = "보통"),
	High		UMETA(DisplayName = "높음"),
	Interrupt	UMETA(DisplayName = "즉시 (강제)")
};

/**
 * FMAITask — AI 태스크 하나를 표현하는 구조체.
 * 큐에 들어가고, 상태 머신에 의해 순차 실행됨.
 */
USTRUCT(BlueprintType)
struct FMAITask
{
	GENERATED_BODY()

	/** 태스크 유형 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	EMTaskType TaskType = EMTaskType::None;

	/** 태스크 카테고리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	EMTaskCategory Category = EMTaskCategory::None;

	/** 현재 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	EMTaskState State = EMTaskState::Pending;

	/** 우선순위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	EMTaskPriority Priority = EMTaskPriority::Normal;

	/** 대상 액터 (공격, 이동, 납치 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	TWeakObjectPtr<AActor> TargetActor;

	/** 대상 위치 (이동, 퇴각 등) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	FVector TargetLocation = FVector::ZeroVector;

	/** 스킬 슬롯 인덱스 (UseSkill 전용, 0~2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	int32 SkillIndex = -1;

	/** 태스크 시작 시각 (GetWorld()->GetTimeSeconds) */
	UPROPERTY(BlueprintReadOnly, Category = "AITask")
	float StartTime = 0.f;

	/** 태스크 제한 시간 (초). 0이면 무제한. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AITask")
	float TimeLimit = 0.f;

	/** Force 계열 태스크인지 판별 */
	bool IsForceTask() const
	{
		return TaskType == EMTaskType::ForceAttack
			|| TaskType == EMTaskType::ForceMove
			|| TaskType == EMTaskType::ForceCarry
			|| TaskType == EMTaskType::ForceRest
			|| TaskType == EMTaskType::ForceSex
			|| TaskType == EMTaskType::ForceMasturbation
			|| Priority == EMTaskPriority::Interrupt;
	}

	/** 유효한 태스크인지 확인 */
	bool IsValid() const { return TaskType != EMTaskType::None; }

	/** 빈 태스크 생성 헬퍼 */
	static FMAITask MakeEmpty() { return FMAITask(); }

	/** 간편 생성 헬퍼 */
	static FMAITask Make(EMTaskType InType, EMTaskCategory InCategory, EMTaskPriority InPriority = EMTaskPriority::Normal)
	{
		FMAITask task;
		task.TaskType = InType;
		task.Category = InCategory;
		task.Priority = InPriority;
		if (task.IsForceTask()) task.Priority = EMTaskPriority::Interrupt;
		return task;
	}

	static FMAITask MakeWithTarget(EMTaskType InType, EMTaskCategory InCategory, AActor* InTarget, EMTaskPriority InPriority = EMTaskPriority::Normal)
	{
		FMAITask task = Make(InType, InCategory, InPriority);
		task.TargetActor = InTarget;
		return task;
	}

	static FMAITask MakeWithLocation(EMTaskType InType, EMTaskCategory InCategory, FVector InLocation, EMTaskPriority InPriority = EMTaskPriority::Normal)
	{
		FMAITask task = Make(InType, InCategory, InPriority);
		task.TargetLocation = InLocation;
		return task;
	}
};
