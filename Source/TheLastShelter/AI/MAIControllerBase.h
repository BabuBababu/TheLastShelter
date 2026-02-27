// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MAIControllerBase.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * MAIControllerBase
 * Eve/Ordo AI 컨트롤러 공통 베이스.
 * AIPerception 설정 및 기본 감지 로직 제공.
 */
UCLASS()
class THELASTSHELTER_API AMAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AMAIControllerBase();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// ---- Perception ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	/** Perception 감지 콜백 */
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	/** 현재 감지된 적 */
	UPROPERTY(BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> DetectedTarget;

public:
	/** 시야 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float SightRadius = 1000.f;

	/** 시야 소실 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float LoseSightRadius = 1200.f;

	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetDetectedTarget() const { return DetectedTarget; }
};
