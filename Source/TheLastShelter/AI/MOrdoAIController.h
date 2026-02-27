// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MAIControllerBase.h"
#include "MOrdoAIController.generated.h"

class AMOrdoCharacter;

/**
 * MOrdoAIController
 * Ordo(적) 전용 AI 컨트롤러.
 * 플레이어와 Eve를 적으로 감지하여 공격.
 */
UCLASS()
class THELASTSHELTER_API AMOrdoAIController : public AMAIControllerBase
{
	GENERATED_BODY()

public:
	AMOrdoAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	TObjectPtr<AMOrdoCharacter> OwnerOrdo;

public:
	/** 공격 사거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float AttackRange = 120.f;

	/** 순찰 범위 (스폰 지점 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float PatrolRadius = 500.f;

	/** 공격 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Ordo")
	float AttackRate = 2.0f;

private:
	FVector SpawnLocation;
	FVector PatrolTarget;
	bool Patrolling = false;
	float LastAttackTime = -999.f;

	void ChooseNewPatrolPoint();
};
