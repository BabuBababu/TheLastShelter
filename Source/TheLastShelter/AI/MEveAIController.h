// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MAIControllerBase.h"
#include "MEveAIController.generated.h"

class AMEveCharacter;

/**
 * MEveAIController
 * Eve(미소녀) 전용 AI 컨트롤러. 플레이어 편.
 * Ordo를 적으로 감지, 플레이어를 따라다님.
 */
UCLASS()
class THELASTSHELTER_API AMEveAIController : public AMAIControllerBase
{
	GENERATED_BODY()

public:
	AMEveAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	/** 소유 Eve 캐싱 */
	UPROPERTY()
	TObjectPtr<AMEveCharacter> OwnerEve;

public:
	/** 플레이어 추적 거리 (이 범위 밖이면 따라감) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float FollowDistance = 300.f;

	/** 전투 감지 시 공격 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float AttackRange = 150.f;

	/** 공격 간격 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float AttackRate = 1.5f;

	/** 플레이어 근처에서 배회 반경 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float WanderRadius = 250.f;

	/** 배회 지점 도달 후 대기 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Eve")
	float WanderIdleTime = 2.0f;

private:
	float LastAttackTime = -999.f;

	// ---- 배회 ----
	FVector WanderTarget = FVector::ZeroVector;
	bool IsWandering = false;
	float WanderIdleEndTime = 0.f;
	bool IsWanderIdle = false;

	void ChooseNewWanderPoint();
};
