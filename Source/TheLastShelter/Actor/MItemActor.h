// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDataTypes.h"
#include "MItemActor.generated.h"

class UPaperSpriteComponent;
class USphereComponent;
class UNiagaraSystem;

/**
 * MItemActor
 * 월드에 떨어진 아이템. 상호작용 시 인벤토리에 추가 후 파괴.
 */
UCLASS()
class THELASTSHELTER_API AMItemActor : public AActor
{
	GENERATED_BODY()

public:
	AMItemActor();

protected:
	virtual void BeginPlay() override;

public:
	/** 아이템 ID로 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeFromData(const FString& InItemID, int32 InCount = 1);

	/** 플레이어/Eve가 상호작용 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void OnPickup(const FString& PickerOwnerID);

	// ---- 데이터 ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemCount = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	FMItemData CachedItemData;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComp;

	/** 픽업 시 재생할 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> PickupVFX;
};
