// Copyright TheLastShelter. All Rights Reserved.

#include "MEveCharacter.h"
#include "MDataManager.h"
#include "MInventoryManager.h"
#include "MOrdoCharacter.h"
#include "MEveAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

AMEveCharacter::AMEveCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AMEveAIController::StaticClass();

	// 2D 캐릭터 회전 방지
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	GetCharacterMovement()->MaxWalkSpeed = 250.f;

	// ---- 공격 VFX 컴포넌트 ----
	AttackVFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackVFX"));
	AttackVFXComp->SetupAttachment(RootComponent);
	AttackVFXComp->bAutoActivate = false;

	// ---- 피격 VFX 컴포넌트 ----
	HitVFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HitVFX"));
	HitVFXComp->SetupAttachment(RootComponent);
	HitVFXComp->bAutoActivate = false;
}

void AMEveCharacter::BeginPlay()
{
	Super::BeginPlay();

	// EveDataID가 설정되어 있으면 데이터에서 초기화
	if (!EveDataID.IsEmpty())
	{
		InitializeFromData(EveDataID);
	}

	// 초기 애니메이션 상태 강제 적용 (Idle 플립북 표시)
	const EMEveAnimState InitState = IsArmed ? EMEveAnimState::GunIdle : EMEveAnimState::Idle;
	CurrentAnimState = EMEveAnimState::MAX; // 강제 갱신을 위해 다른 값으로 초기화
	SetAnimState(InitState);
}

void AMEveCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 액션 애니메이션 타이머 해제
	if (IsPlayingActionAnim && GetWorld()->GetTimeSeconds() >= ActionAnimEndTime)
	{
		IsPlayingActionAnim = false;
		IsAttacking = false;
	}

	// 이동 기반 상태 전환 (AI가 이동을 결정, 여기서 비주얼만 갱신)
	UpdateAnimStateFromMovement();
}

void AMEveCharacter::InitializeFromData(const FString& EveID)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	FMEveData Data;
	if (DataMgr->GetEveDataByID(EveID, Data))
	{
		EveDataID = Data.ID;
		EveName = Data.Name;
		PhysicalStat = Data.PhysicalStat;
		MentalStat = Data.MentalStat;
		Affection = Data.Affection;
		HiddenStats = Data.HiddenStats;
		SkillIDs = Data.SkillIDs;
		CurrentHealth = PhysicalStat.Health;

		// MoveSpeed 적용
		GetCharacterMovement()->MaxWalkSpeed = PhysicalStat.MoveSpeed;

		// 인벤토리 매니저에 이 Eve의 인벤토리 생성
		UMInventoryManager* InvMgr = GI->GetSubsystem<UMInventoryManager>();
		if (InvMgr && !InvMgr->HasInventory(EveDataID))
		{
			InvMgr->CreateInventory(EveDataID, 20);
		}

		UE_LOG(LogTemp, Log, TEXT("[Eve] Initialized: %s (%s)"), *EveName, *EveDataID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Eve] Failed to load data for ID: %s"), *EveID);
	}
}

void AMEveCharacter::SetEmotion(EMEmotion NewEmotion)
{
	CurrentEmotion = NewEmotion;

	// TODO: PaperZD AnimInstance에 감정 변수 전달 → 애니메이션 분기
	UE_LOG(LogTemp, Log, TEXT("[Eve] %s emotion changed to %d"), *EveName, static_cast<int32>(NewEmotion));
}

void AMEveCharacter::AssignRandomHiddenStats()
{
	// 1~3개 랜덤 히든 스탯 부여
	const int32 Count = FMath::RandRange(1, 3);
	TArray<EMHiddenStatType> AllTypes;
	for (int32 i = static_cast<int32>(EMHiddenStatType::StrongWill);
		 i <= static_cast<int32>(EMHiddenStatType::Agile); ++i)
	{
		AllTypes.Add(static_cast<EMHiddenStatType>(i));
	}

	// 셔플 후 앞에서 Count개 선택
	for (int32 i = AllTypes.Num() - 1; i > 0; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		AllTypes.Swap(i, j);
	}

	HiddenStats.Empty();
	for (int32 i = 0; i < FMath::Min(Count, AllTypes.Num()); ++i)
	{
		HiddenStats.Add(AllTypes[i]);
	}

	UE_LOG(LogTemp, Log, TEXT("[Eve] %s received %d hidden stats"), *EveName, HiddenStats.Num());
}

void AMEveCharacter::TakeDamageFromOrdo(float Damage)
{
	const float ActualDamage = FMath::Max(0.f, Damage - PhysicalStat.Defense);
	CurrentHealth -= ActualDamage;
	CurrentHealth = FMath::Max(0.f, CurrentHealth);

	PlayHitVFX();

	UE_LOG(LogTemp, Log, TEXT("[Eve] %s took %.1f damage (%.1f HP left)"), *EveName, ActualDamage, CurrentHealth);

	if (IsDead())
	{
		SetAnimState(EMEveAnimState::Down);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 99999.f; // 사망 상태 유지
		UE_LOG(LogTemp, Log, TEXT("[Eve] %s is dead!"), *EveName);
	}
	else
	{
		SetAnimState(EMEveAnimState::Hit);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 0.3f; // 피격 경직
	}
}

// ============================================================
// 공격
// ============================================================

void AMEveCharacter::PerformAttack(AActor* Target)
{
	if (IsDead() || !Target || IsPlayingActionAnim) return;

	// 타겟 방향 기반 좌/우 공격 판정
	const float dirX = Target->GetActorLocation().X - GetActorLocation().X;
	if (dirX >= 0.f)
		SetAnimState(EMEveAnimState::GunShot_Right);
	else
		SetAnimState(EMEveAnimState::GunShot_Left);

	PlayAttackVFX();

	IsAttacking = true;
	IsPlayingActionAnim = true;
	ActionAnimEndTime = GetWorld()->GetTimeSeconds() + ActionAnimDuration;

	// 데미지 적용
	if (AMOrdoCharacter* Ordo = Cast<AMOrdoCharacter>(Target))
	{
		Ordo->TakeDamageFromPlayer(PhysicalStat.Attack);
	}
}

// ============================================================
// 이동 기반 상태 자동 전환
// ============================================================

void AMEveCharacter::UpdateAnimStateFromMovement()
{
	if (IsPlayingActionAnim || IsDead()) return;

	const FVector vel = GetVelocity();
	const float speed = vel.Size2D();

	// 정지 → Idle
	if (speed < 10.f)
	{
		SetAnimState(IsArmed ? EMEveAnimState::GunIdle : EMEveAnimState::Idle);
		return;
	}

	// 방향 판정 (X=좌우, Y=상하)
	const bool isHorizontalDominant = FMath::Abs(vel.X) >= FMath::Abs(vel.Y);
	const bool isRunning = speed >= RunSpeedThreshold;

	if (IsArmed)
	{
		if (isRunning)
		{
			if (isHorizontalDominant)
				SetAnimState(vel.X > 0 ? EMEveAnimState::GunRun_Right : EMEveAnimState::GunRun_Left);
			else
				SetAnimState(vel.Y > 0 ? EMEveAnimState::GunRun_Down : EMEveAnimState::GunRun_Up);
		}
		else
		{
			if (isHorizontalDominant)
				SetAnimState(vel.X > 0 ? EMEveAnimState::GunWalk_Right : EMEveAnimState::GunWalk_Left);
			else
				SetAnimState(vel.Y > 0 ? EMEveAnimState::GunWalk_Down : EMEveAnimState::GunWalk_Up);
		}
	}
	else
	{
		if (isRunning)
		{
			if (isHorizontalDominant)
				SetAnimState(vel.X > 0 ? EMEveAnimState::Run_Right : EMEveAnimState::Run_Left);
			else
				SetAnimState(vel.Y > 0 ? EMEveAnimState::Run_Down : EMEveAnimState::Run_Up);
		}
		else
		{
			if (isHorizontalDominant)
				SetAnimState(vel.X > 0 ? EMEveAnimState::Walk_Right : EMEveAnimState::Walk_Left);
			else
				SetAnimState(vel.Y > 0 ? EMEveAnimState::Walk_Down : EMEveAnimState::Walk_Up);
		}
	}
}

// ============================================================
// 애니메이션 상태 전환
// ============================================================

void AMEveCharacter::SetAnimState(EMEveAnimState NewState)
{
	if (CurrentAnimState == NewState) return;

	CurrentAnimState = NewState;

	// 매핑 테이블에서 해당 상태의 플립북 에셋을 찾아 교체
	const TSoftObjectPtr<UPaperFlipbook>* FoundFlipbook = AnimFlipbookMap.Find(NewState);
	if (FoundFlipbook && !FoundFlipbook->IsNull())
	{
		UPaperFlipbook* loadedFlipbook = FoundFlipbook->LoadSynchronous();
		if (loadedFlipbook && GetSprite())
		{
			GetSprite()->SetFlipbook(loadedFlipbook);
			GetSprite()->PlayFromStart();
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[Eve] %s anim state → %d"), *EveName, static_cast<int32>(NewState));
}

// ============================================================
// VFX
// ============================================================

void AMEveCharacter::PlayAttackVFX()
{
	if (AttackVFXComp && AttackVFXAsset)
	{
		AttackVFXComp->SetAsset(AttackVFXAsset);
		AttackVFXComp->Activate(true);
	}
}

void AMEveCharacter::PlayHitVFX()
{
	if (HitVFXComp && HitVFXAsset)
	{
		HitVFXComp->SetAsset(HitVFXAsset);
		HitVFXComp->Activate(true);
	}
}
