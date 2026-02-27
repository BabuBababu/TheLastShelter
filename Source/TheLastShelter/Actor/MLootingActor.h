// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDataTypes.h"
#include "MLootingActor.generated.h"

class UPaperSpriteComponent;
class USphereComponent;
class UNiagaraSystem;
class AMItemActor;

/**
 * MLootingActor
 * 루팅 오브젝트. 상호작용 시 MItemActor를 스폰하고 자신은 파괴.
 */
UCLASS()
class THELASTSHELTER_API AMLootingActor : public AActor
{
	GENERATED_BODY()

public:
	AMLootingActor();

protected:
	virtual void BeginPlay() override;

public:
	/** 루팅 오브젝트 ID로 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Looting")
	void InitializeFromData(const FString& InLootingID);

	/** 플레이어/Eve가 상호작용 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Looting")
	void OnInteract();

	// ---- 데이터 ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Looting")
	FString LootingObjectID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Looting")
	FMLootingObjectData CachedLootingData;

	/** 스폰할 MItemActor 블루프린트 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Looting")
	TSubclassOf<AMItemActor> ItemActorClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComp;

	/** 파괴 시 재생할 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> DestroyVFX;
};
