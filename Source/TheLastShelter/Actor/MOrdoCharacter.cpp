// Copyright TheLastShelter. All Rights Reserved.

#include "MOrdoCharacter.h"
#include "MDataManager.h"
#include "MEveCharacter.h"
#include "MOrdoAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

AMOrdoCharacter::AMOrdoCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AMOrdoAIController::StaticClass();

	// 2D 캐릭터 회전 방지
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	GetCharacterMovement()->MaxWalkSpeed = 200.f;

	// ---- 공격 VFX 컴포넌트 ----
	AttackVFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackVFX"));
	AttackVFXComp->SetupAttachment(RootComponent);
	AttackVFXComp->bAutoActivate = false;

	// ---- 피격 VFX 컴포넌트 ----
	HitVFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HitVFX"));
	HitVFXComp->SetupAttachment(RootComponent);
	HitVFXComp->bAutoActivate = false;
}

void AMOrdoCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!OrdoDataID.IsEmpty())
	{
		InitializeFromData(OrdoDataID);
	}

	// 초기 애니메이션 상태 강제 적용 (GunIdle 플립북 표시)
	CurrentAnimState = EMOrdoAnimState::MAX;
	SetAnimState(EMOrdoAnimState::GunIdle);
}

void AMOrdoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 액션 애니메이션 타이머 해제
	if (IsPlayingActionAnim && GetWorld()->GetTimeSeconds() >= ActionAnimEndTime)
	{
		IsPlayingActionAnim = false;
		IsAttacking = false;
	}

	// 이동 기반 상태 전환
	UpdateAnimStateFromMovement();
}

void AMOrdoCharacter::InitializeFromData(const FString& OrdoID)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	FMOrdoData Data;
	if (DataMgr->GetOrdoDataByID(OrdoID, Data))
	{
		OrdoDataID = Data.ID;
		OrdoName = Data.Name;
		OrdoType = Data.OrdoType;
		PhysicalStat = Data.PhysicalStat;
		SkillIDs = Data.SkillIDs;
		DropTableID = Data.DropTableID;
		CurrentHealth = PhysicalStat.Health;

		GetCharacterMovement()->MaxWalkSpeed = PhysicalStat.MoveSpeed;

		UE_LOG(LogTemp, Log, TEXT("[Ordo] Initialized: %s (%s)"), *OrdoName, *OrdoDataID);
	}
}

void AMOrdoCharacter::TakeDamageFromPlayer(float Damage)
{
	const float ActualDamage = FMath::Max(0.f, Damage - PhysicalStat.Defense);
	CurrentHealth -= ActualDamage;
	CurrentHealth = FMath::Max(0.f, CurrentHealth);

	PlayHitVFX();

	UE_LOG(LogTemp, Log, TEXT("[Ordo] %s took %.1f damage (%.1f HP left)"), *OrdoName, ActualDamage, CurrentHealth);

	if (IsDead())
	{
		SetAnimState(EMOrdoAnimState::Down);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 99999.f; // 사망 상태 유지
		SpawnDropItems();
	}
	else
	{
		SetAnimState(EMOrdoAnimState::Hit);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 0.3f; // 피격 경직
	}
}

void AMOrdoCharacter::SpawnDropItems()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	FMDropTableData DropTable;
	if (!DataMgr->GetDropTableByID(DropTableID, DropTable)) return;

	for (const FMDropItemEntry& Entry : DropTable.Entries)
	{
		// 확률 체크
		if (FMath::FRand() > Entry.DropRate) continue;

		const int32 SpawnCount = FMath::RandRange(Entry.MinCount, Entry.MaxCount);
		if (SpawnCount <= 0) continue;

		// MItemActor 스폰 (TODO: 실제 MItemActor 클래스 지정)
		UE_LOG(LogTemp, Log, TEXT("[Ordo] %s drops item: %s x%d"), *OrdoName, *Entry.ItemID, SpawnCount);
		// TODO: GetWorld()->SpawnActor<AMItemActor>(...)
	}
}

// ============================================================
// 공격
// ============================================================

void AMOrdoCharacter::PerformAttack(AActor* Target)
{
	if (IsDead() || !Target || IsPlayingActionAnim) return;

	// 타겟 방향 기반 좌/우 공격 판정
	const float dirX = Target->GetActorLocation().X - GetActorLocation().X;
	if (dirX >= 0.f)
		SetAnimState(EMOrdoAnimState::GunShot_Right);
	else
		SetAnimState(EMOrdoAnimState::GunShot_Left);

	PlayAttackVFX();

	IsAttacking = true;
	IsPlayingActionAnim = true;
	ActionAnimEndTime = GetWorld()->GetTimeSeconds() + ActionAnimDuration;

	// 데미지 적용
	if (AMEveCharacter* Eve = Cast<AMEveCharacter>(Target))
	{
		Eve->TakeDamageFromOrdo(PhysicalStat.Attack);
	}
	// TODO: AMPlayerCharacter에대한 데미지 처리
}

// ============================================================
// 이동 기반 상태 자동 전환
// ============================================================

void AMOrdoCharacter::UpdateAnimStateFromMovement()
{
	if (IsPlayingActionAnim || IsDead()) return;

	const FVector vel = GetVelocity();
	const float speed = vel.Size2D();

	// 정지 → Idle
	if (speed < 10.f)
	{
		SetAnimState(EMOrdoAnimState::GunIdle);
		return;
	}

	// 방향 판정 (X=좌우, Y=상하)
	const bool isHorizontalDominant = FMath::Abs(vel.X) >= FMath::Abs(vel.Y);
	const bool isRunning = speed >= RunSpeedThreshold;

	if (isRunning)
	{
		if (isHorizontalDominant)
			SetAnimState(vel.X > 0 ? EMOrdoAnimState::GunRun_Right : EMOrdoAnimState::GunRun_Left);
		else
			SetAnimState(vel.Y > 0 ? EMOrdoAnimState::GunRun_Down : EMOrdoAnimState::GunRun_Up);
	}
	else
	{
		if (isHorizontalDominant)
			SetAnimState(vel.X > 0 ? EMOrdoAnimState::GunWalk_Right : EMOrdoAnimState::GunWalk_Left);
		else
			SetAnimState(vel.Y > 0 ? EMOrdoAnimState::GunWalk_Down : EMOrdoAnimState::GunWalk_Up);
	}
}

// ============================================================
// 애니메이션 상태 전환
// ============================================================

void AMOrdoCharacter::SetAnimState(EMOrdoAnimState NewState)
{
	if (CurrentAnimState == NewState) return;

	CurrentAnimState = NewState;

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

	UE_LOG(LogTemp, Verbose, TEXT("[Ordo] %s anim state → %d"), *OrdoName, static_cast<int32>(NewState));
}

// ============================================================
// VFX
// ============================================================

void AMOrdoCharacter::PlayAttackVFX()
{
	if (AttackVFXComp && AttackVFXAsset)
	{
		AttackVFXComp->SetAsset(AttackVFXAsset);
		AttackVFXComp->Activate(true);
	}
}

void AMOrdoCharacter::PlayHitVFX()
{
	if (HitVFXComp && HitVFXAsset)
	{
		HitVFXComp->SetAsset(HitVFXAsset);
		HitVFXComp->Activate(true);
	}
}
