// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MBaseTask.h"
#include "MKidnapTask.generated.h"

class AMEveCharacter;

/**
 * UMKidnapTask — 납치 태스크 (Ordo 전용).
 *
 * 1단계: 기절한 Eve에게 접근
 * 2단계: Eve를 엎고 퇴각 (Retreat 태스크 생성)
 *
 * Eve가 아직 기절하지 않았으면 Attack처럼 공격.
 */
UCLASS()
class THELASTSHELTER_API UMKidnapTask : public UMBaseTask
{
	GENERATED_BODY()

public:
	UMKidnapTask();

	float GrabDistance = 80.f;
	FVector RetreatDestination = FVector::ZeroVector;

	virtual void StartTask() override;
	virtual void TickTask(float DeltaTime) override;

private:
	bool bGrabbed = false;
};
