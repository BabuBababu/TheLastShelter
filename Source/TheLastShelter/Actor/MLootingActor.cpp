// Copyright TheLastShelter. All Rights Reserved.

#include "MLootingActor.h"
#include "MItemActor.h"
#include "MDataManager.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

AMLootingActor::AMLootingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(50.f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(CollisionComp);

	SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SpriteComp->SetupAttachment(RootComponent);

	Tags.Add(TEXT("Interactable"));
	Tags.Add(TEXT("Looting"));
}

void AMLootingActor::BeginPlay()
{
	Super::BeginPlay();

	if (!LootingObjectID.IsEmpty())
	{
		InitializeFromData(LootingObjectID);
	}
}

void AMLootingActor::InitializeFromData(const FString& InLootingID)
{
	LootingObjectID = InLootingID;

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	if (DataMgr->GetLootingObjectDataByID(LootingObjectID, CachedLootingData))
	{
		// TODO: SpritePath로 UPaperSprite 동적 로드
		UE_LOG(LogTemp, Log, TEXT("[LootingActor] Initialized: %s"), *CachedLootingData.Name);
	}
}

void AMLootingActor::OnInteract()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 등록된 아이템 ID들로 MItemActor 스폰
	for (const FString& SpawnItemID : CachedLootingData.SpawnItemIDs)
	{
		if (ItemActorClass)
		{
			// 랜덤 오프셋으로 흩뿌리기
			const FVector Offset(FMath::RandRange(-50.f, 50.f), FMath::RandRange(-50.f, 50.f), 0.f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AMItemActor* SpawnedItem = World->SpawnActor<AMItemActor>(
				ItemActorClass, GetActorLocation() + Offset, FRotator::ZeroRotator, SpawnParams);

			if (SpawnedItem)
			{
				SpawnedItem->InitializeFromData(SpawnItemID, 1);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LootingActor] ItemActorClass not set. Cannot spawn item: %s"), *SpawnItemID);
		}
	}

	// 파괴 VFX 재생
	if (DestroyVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, DestroyVFX, GetActorLocation(), FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Log, TEXT("[LootingActor] %s destroyed after interaction"), *CachedLootingData.Name);
	Destroy();
}
