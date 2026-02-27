// Copyright TheLastShelter. All Rights Reserved.

#include "MItemActor.h"
#include "MDataManager.h"
#include "MInventoryManager.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

AMItemActor::AMItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 충돌 컴포넌트
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetSphereRadius(30.f);
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetRootComponent(CollisionComp);

	// 스프라이트 비주얼
	SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Sprite"));
	SpriteComp->SetupAttachment(RootComponent);

	// 상호작용 태그
	Tags.Add(TEXT("Interactable"));
	Tags.Add(TEXT("Item"));
}

void AMItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemID.IsEmpty())
	{
		InitializeFromData(ItemID, ItemCount);
	}
}

void AMItemActor::InitializeFromData(const FString& InItemID, int32 InCount)
{
	ItemID = InItemID;
	ItemCount = FMath::Max(1, InCount);

	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	if (DataMgr->GetItemDataByID(ItemID, CachedItemData))
	{
		// TODO: SpritePath를 사용하여 UPaperSprite 에셋을 동적 로드 후 SpriteComp에 설정
		UE_LOG(LogTemp, Log, TEXT("[ItemActor] Initialized: %s x%d"), *CachedItemData.Name, ItemCount);
	}
}

void AMItemActor::OnPickup(const FString& PickerOwnerID)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMInventoryManager* InvMgr = GI->GetSubsystem<UMInventoryManager>();
	if (!InvMgr) return;

	const int32 Remaining = InvMgr->AddItem(PickerOwnerID, ItemID, ItemCount);

	if (Remaining > 0)
	{
		// 인벤토리 꽉 참 → 남은 수량으로 업데이트
		ItemCount = Remaining;
		UE_LOG(LogTemp, Warning, TEXT("[ItemActor] Inventory full for %s. %d items remain."), *PickerOwnerID, Remaining);
		return;
	}

	// 픽업 VFX 재생
	if (PickupVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), PickupVFX, GetActorLocation(), FRotator::ZeroRotator);
	}

	UE_LOG(LogTemp, Log, TEXT("[ItemActor] %s picked up %s x%d"), *PickerOwnerID, *CachedItemData.Name, ItemCount);
	Destroy();
}
