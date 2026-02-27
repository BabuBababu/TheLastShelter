// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	GunShot				UMETA(DisplayName = "총 사격"),
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
	GunShot				UMETA(DisplayName = "총 사격"),
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
struct FMEveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	FString ID;

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
	TArray<FString> SkillIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve")
	FString SpritePath;
};

/** Ordo 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMOrdoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	EMOrdoType OrdoType = EMOrdoType::Axiom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FMPhysicalStat PhysicalStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	TArray<FString> SkillIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FString DropTableID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo")
	FString SpritePath;
};

/** Primal 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMPrimalData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	EMPrimalType PrimalType = EMPrimalType::Animal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	FString DropTableID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Primal")
	FString SpritePath;
};

/** Item 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ID;

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
struct FMStatDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	bool IsPhysical = true;
};

/** HiddenStat 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMHiddenStatData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HiddenStat")
	FString ID;

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
struct FMSkillData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FString ID;

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
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	float DropRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MinCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct FMDropTableData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
	TArray<FMDropItemEntry> Entries;
};

/** Event 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMEventData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EMEventType EventType = EMEventType::RestVisit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	float TriggerProbability = 1.f;
};

/** SpecialEvent 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMSpecialEventData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialEvent")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialEvent")
	FString Title;

	/** 대사 스크립트 ('{EveName}'이 실제 이브 이름으로 치환) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpecialEvent")
	TArray<FString> ScriptLines;
};

/** EveDialog 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMEveDialogData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString EveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	EMEmotion Emotion = EMEmotion::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	EMEventType EventCondition = EMEventType::RestVisit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString DialogText;
};

/** OrdoDialog 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMOrdoDialogData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString OrdoID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString DialogText;
};

/** PlayerDialog 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMPlayerDialogData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	EMEventType EventCondition = EMEventType::RestVisit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialog")
	FString DialogText;
};

/** LootingObject 테이블 데이터 */
USTRUCT(BlueprintType)
struct FMLootingObjectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	FString Name;

	/** 루팅 시 스폰할 아이템 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	TArray<FString> SpawnItemIDs;

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
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Count = 0;

	bool IsEmpty() const { return ItemID.IsEmpty() || Count <= 0; }
};
