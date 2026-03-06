// Copyright TheLastShelter. All Rights Reserved.

#include "MAIControllerBase.h"
#include "MAITaskComponent.h"
#include "MBaseTask.h"
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

	// 컨트롤러 캐싱 + 콜백 바인딩
	if (TaskComp)
	{
		TaskComp->SetOwnerController(this);
		TaskComp->OnTaskStarted.AddDynamic(this, &AMAIControllerBase::HandleTaskStarted);
	}
}

void AMAIControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn()) return;

	// 태스크 컴포넌트가 CurrentTask를 구동.
	// 컨트롤러는 Idle 행동만 처리.
	if (!TaskComp || TaskComp->IsIdle())
	{
		ExecuteIdleBehavior(DeltaTime);
	}
}

// ---- 가상 함수 기본 구현 ----

UMBaseTask* AMAIControllerBase::CreateTaskForType(EMTaskType TaskType, AActor* Target, const FVector& Location)
{
	// 기본: nullptr. 하위 클래스에서 오버라이드.
	return nullptr;
}

void AMAIControllerBase::ExecuteIdleBehavior(float DeltaTime)
{
	// 기본: 정지
}

void AMAIControllerBase::OnNewTaskStarted(UMBaseTask* Task)
{
	// 기본: 이동 정지 (하위 클래스에서 확장)
	StopMovement();
}

AActor* AMAIControllerBase::ResolveAttackTarget() const
{
	// 기본: DetectedTarget 반환. 하위 클래스에서 오버라이드하여 LastAttacker 우선 로직 추가.
	return DetectedTarget;
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

	UMBaseTask* task = CreateTaskForType(TaskType, Target, Location);
	if (!task)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AIBase] CreateTaskForType returned null for %s"),
			*UEnum::GetValueAsString(TaskType));
		return;
	}

	task->Category = Category;
	task->Priority = Priority;
	if (task->IsForceTask()) task->Priority = EMTaskPriority::Interrupt;

	TaskComp->EnqueueTask(task);
}

void AMAIControllerBase::ForceTask(EMTaskType TaskType, EMTaskCategory Category,
	AActor* Target, FVector Location)
{
	if (!TaskComp) return;

	UMBaseTask* task = CreateTaskForType(TaskType, Target, Location);
	if (!task) return;

	task->Category = Category;
	task->Priority = EMTaskPriority::Interrupt;

	TaskComp->InterruptWithTask(task);
}

void AMAIControllerBase::ClearAllTasks()
{
	if (!TaskComp) return;
	TaskComp->ClearAllTasks();
	StopMovement();
}

void AMAIControllerBase::HandleTaskStarted(UMBaseTask* Task)
{
	OnNewTaskStarted(Task);
}
