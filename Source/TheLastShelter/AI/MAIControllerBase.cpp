// Copyright TheLastShelter. All Rights Reserved.

#include "MAIControllerBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AMAIControllerBase::AMAIControllerBase()
{
	// Perception 컴포넌트 생성
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	// 시각 감지 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->SetMaxAge(5.f);

	// 적/아군 모두 감지 (하위 클래스에서 필터링)
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AMAIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	// Perception 업데이트 바인딩
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AMAIControllerBase::OnPerceptionUpdated);
	}
}

void AMAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Log, TEXT("[AIBase] Possessed: %s"), *InPawn->GetName());
}

void AMAIControllerBase::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// 기본 구현: 가장 가까운 감지 액터를 타겟으로 설정
	// 하위 클래스에서 오버라이드하여 적/아군 구분

	float MinDist = MAX_FLT;
	AActor* Nearest = nullptr;

	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor || Actor == GetPawn()) continue;

		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComp->GetActorsPerception(Actor, Info);

		// 현재 감지중인지 확인
		bool bCurrentlySensed = false;
		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				bCurrentlySensed = true;
				break;
			}
		}

		if (bCurrentlySensed)
		{
			const float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
			if (Dist < MinDist)
			{
				MinDist = Dist;
				Nearest = Actor;
			}
		}
	}

	DetectedTarget = Nearest;
}
