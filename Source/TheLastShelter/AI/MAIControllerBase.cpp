// Copyright TheLastShelter. All Rights Reserved.

#include "MAIControllerBase.h"
#include "MAITaskComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AMAIControllerBase::AMAIControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Perception 컴포넌트 생성
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	// 시각 감지 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->SetMaxAge(5.f);

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AMAIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AMAIControllerBase::OnPerceptionUpdated);
	}
}

void AMAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Log, TEXT("[AIBase] Possessed: %s"), *InPawn->GetName());

	// TaskComponent 탐색/생성
	TaskComp = InPawn->FindComponentByClass<UMAITaskComponent>();
	if (!TaskComp)
	{
		TaskComp = NewObject<UMAITaskComponent>(InPawn, TEXT("AITaskComp"));
		TaskComp->RegisterComponent();
		UE_LOG(LogTemp, Log, TEXT("[AIBase] Created AITaskComponent on %s"), *InPawn->GetName());
	}

	// 태스크 시작 콜백 바인딩
	if (TaskComp)
	{
		TaskComp->OnTaskStarted.AddDynamic(this, &AMAIControllerBase::HandleTaskStarted);
	}
}

void AMAIControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn()) return;

	// 태스크 구동
	if (TaskComp && TaskComp->HasCurrentTask())
	{
		ExecuteTask(DeltaTime, TaskComp->GetCurrentTask());
	}
	else
	{
		ExecuteIdleBehavior(DeltaTime);
	}
}

// ---- 가상 함수 기본 구현 ----

void AMAIControllerBase::ExecuteTask(float DeltaTime, const FMAITask& Task)
{
	// 기본: 아무것도 안함. 하위 클래스에서 오버라이드.
}

void AMAIControllerBase::ExecuteIdleBehavior(float DeltaTime)
{
	// 기본: 정지
}

void AMAIControllerBase::OnTaskBegin(const FMAITask& Task)
{
	// 하위 클래스에서 초기화용
}

// ---- Perception ----

void AMAIControllerBase::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	float minDist = MAX_FLT;
	AActor* nearest = nullptr;

	for (AActor* actor : UpdatedActors)
	{
		if (!actor || actor == GetPawn()) continue;

		FActorPerceptionBlueprintInfo info;
		AIPerceptionComp->GetActorsPerception(actor, info);

		bool currentlySensed = false;
		for (const FAIStimulus& stimulus : info.LastSensedStimuli)
		{
			if (stimulus.WasSuccessfullySensed())
			{
				currentlySensed = true;
				break;
			}
		}

		if (currentlySensed)
		{
			const float dist = FVector::Dist(GetPawn()->GetActorLocation(), actor->GetActorLocation());
			if (dist < minDist)
			{
				minDist = dist;
				nearest = actor;
			}
		}
	}

	DetectedTarget = nearest;
}

// ---- 태스크 편의 API ----

void AMAIControllerBase::RequestTask(EMTaskType TaskType, EMTaskCategory Category,
	AActor* Target, FVector Location, EMTaskPriority Priority)
{
	if (!TaskComp) return;
	TaskComp->RequestTask(TaskType, Category, Target, Location, Priority);
}

void AMAIControllerBase::ForceTask(EMTaskType TaskType, EMTaskCategory Category,
	AActor* Target, FVector Location)
{
	if (!TaskComp) return;
	FMAITask task = FMAITask::MakeWithTarget(TaskType, Category, Target, EMTaskPriority::Interrupt);
	task.TargetLocation = Location;
	TaskComp->InterruptWithTask(task);
}

void AMAIControllerBase::ClearAllTasks()
{
	if (!TaskComp) return;
	TaskComp->ClearAllTasks();
	StopMovement();
}

void AMAIControllerBase::HandleTaskStarted(const FMAITask& Task)
{
	OnTaskBegin(Task);
}
