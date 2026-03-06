// Copyright TheLastShelter. All Rights Reserved.

#include "MOrdoCharacter.h"
#include "MStatComponent.h"
#include "MDataManager.h"
#include "MProjectileManager.h"
#include "MEveCharacter.h"
#include "MOrdoAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"

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

	// ---- 스탯 컴포넌트 ----
	StatComp = CreateDefaultSubobject<UMStatComponent>(TEXT("StatComp"));

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

	if (OrdoDataId > 0)
	{
		InitializeFromData(OrdoDataId);
	}

	// 총알 BP 클래스를 MProjectileManager에 자동 등록
	if (BulletClass)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMProjectileManager* ProjMgr = GI->GetSubsystem<UMProjectileManager>())
			{
				// BulletClass 풀 사전 워밍
				ProjMgr->EnsurePool(GetWorld(), BulletClass);
			}
		}
	}

	// 초기 애니메이션 상태 강제 적용 (GunIdle 플립북 표시)
	CurrentAnimState = EMOrdoAnimState::MAX;
	SetAnimState(EMOrdoAnimState::GunIdle);

	// 스프라이트 기본 스케일 저장 (전투 스케일 복원에 사용)
	if (GetSprite())
	{
		DefaultSpriteScale = GetSprite()->GetRelativeScale3D();
	}
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

void AMOrdoCharacter::InitializeFromData(int32 OrdoId)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	FMOrdoData Data;
	if (DataMgr->GetOrdoDataByID(OrdoId, Data))
	{
		OrdoDataId = Data.Id;
		OrdoName = Data.Name;
		OrdoType = Data.OrdoType;
		SkillIDs = Data.SkillIDs;
		DropTableID = Data.DropTableID;

		if (StatComp)
		{
			StatComp->InitializeFromPhysicalStat(Data.PhysicalStat);
		}

		GetCharacterMovement()->MaxWalkSpeed = Data.PhysicalStat.MoveSpeed;

		UE_LOG(LogTemp, Log, TEXT("[Ordo] Initialized: %s (Id=%d)"), *OrdoName, OrdoDataId);
	}
}

void AMOrdoCharacter::TakeDamageFromPlayer(float Damage, AActor* Attacker)
{
	if (!StatComp) return;

	// 공격자를 AI 컨트롤러에 알림 → 반격 우선 타겟 설정
	if (Attacker)
	{
		if (auto* AICtrl = Cast<AMAIControllerBase>(GetController()))
		{
			AICtrl->SetLastAttacker(Attacker);
		}
	}

	float actualDamage = StatComp->ApplyDamage(Damage);
	PlayHitVFX();

	UE_LOG(LogTemp, Log, TEXT("[Ordo] %s took %.1f damage (%.1f HP left)"),
		*OrdoName, actualDamage, StatComp->GetCurrentHealth());

	if (StatComp->IsDead())
	{
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 99999.f;

		// Down 플립북이 있으면 1회 재생 후 완료 델리게이트로 Hidden,
		// 없으면 즉시 Hidden.
		const TSoftObjectPtr<UPaperFlipbook>* downFlipbook = AnimFlipbookMap.Find(EMOrdoAnimState::Down);
		if (downFlipbook && !downFlipbook->IsNull())
		{
			UPaperFlipbook* loadedFB = downFlipbook->LoadSynchronous();
			if (loadedFB && GetSprite())
			{
				GetSprite()->SetFlipbook(loadedFB);
				GetSprite()->SetLooping(false);
				GetSprite()->PlayFromStart();
				CurrentAnimState = EMOrdoAnimState::Down;

				// 플립북 재생 완료 델리게이트
				GetSprite()->OnFinishedPlaying.AddDynamic(this, &AMOrdoCharacter::OnDownAnimFinished);

				UE_LOG(LogTemp, Log, TEXT("[Ordo] %s Down 플립북 재생 시작"), *OrdoName);
			}
			else
			{
				// 로드 실패 → 즉시 Hidden
				SetActorHiddenInGame(true);
			}
		}
		else
		{
			// Down 플립북 없음 → 즉시 Hidden
			SetActorHiddenInGame(true);
		}

		SpawnDropItems();
	}
	else
	{
		SetAnimState(EMOrdoAnimState::Hit);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 0.3f;
	}
}

bool AMOrdoCharacter::IsDead() const
{
	return StatComp ? StatComp->IsDead() : true;
}

void AMOrdoCharacter::OnDownAnimFinished()
{
	// 델리게이트 해제 (풀 재사용 시 재바인딩 방지)
	if (GetSprite())
	{
		GetSprite()->OnFinishedPlaying.RemoveDynamic(this, &AMOrdoCharacter::OnDownAnimFinished);
	}

	SetActorHiddenInGame(true);
	UE_LOG(LogTemp, Log, TEXT("[Ordo] %s Down 애니메이션 완료 → Hidden"), *OrdoName);
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
		UE_LOG(LogTemp, Log, TEXT("[Ordo] %s drops item Id: %d x%d"), *OrdoName, Entry.ItemId, SpawnCount);
		// TODO: GetWorld()->SpawnActor<AMItemActor>(...)
	}
}

// ============================================================
// 공격
// ============================================================

void AMOrdoCharacter::PerformAttack(AActor* Target)
{
	if (IsDead() || !Target || IsPlayingActionAnim) return;

	// CombatLoop 상태 + FaceTarget으로 방향 처리되므로 별도 애니메이션 상태 전환 없음

	PlayAttackVFX();

	IsAttacking = true;
	IsPlayingActionAnim = true;
	ActionAnimEndTime = GetWorld()->GetTimeSeconds() + ActionAnimDuration;

	// 총알 발사 (MProjectileManager 경유)
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMProjectileManager* projMgr = GI->GetSubsystem<UMProjectileManager>())
		{
			FVector muzzleOffset = FVector(0.f, 0.f, 10.f);
			projMgr->FireBullet(this, Target, muzzleOffset);
		}
	}
	// TODO: AMPlayerCharacter에대한 데미지 처리
}

// ============================================================
// 이동 기반 상태 자동 전환
// ============================================================

void AMOrdoCharacter::UpdateAnimStateFromMovement()
{
	if (IsPlayingActionAnim || IsDead()) return;

	// CombatEnter / CombatLoop / CombatExit — AI가 직접 관리하므로 항상 보호
	if (CurrentAnimState == EMOrdoAnimState::CombatEnter ||
		CurrentAnimState == EMOrdoAnimState::CombatLoop ||
		CurrentAnimState == EMOrdoAnimState::CombatExit)
	{
		return;
	}

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

	const EMOrdoAnimState OldState = CurrentAnimState;
	CurrentAnimState = NewState;

	const TSoftObjectPtr<UPaperFlipbook>* FoundFlipbook = AnimFlipbookMap.Find(NewState);
	if (FoundFlipbook && !FoundFlipbook->IsNull())
	{
		UPaperFlipbook* loadedFlipbook = FoundFlipbook->LoadSynchronous();
		if (loadedFlipbook && GetSprite())
		{
			GetSprite()->SetFlipbook(loadedFlipbook);
			// 역재생 상태에서 전환 시 PlayRate 정방향 복원
			if (GetSprite()->GetPlayRate() < 0.f)
			{
				GetSprite()->SetPlayRate(1.0f);
			}
			GetSprite()->PlayFromStart();
		}
	}

	// --- 전투 스프라이트 스케일 적용/복원 ---
	auto IsCombatAnim = [](EMOrdoAnimState s)
	{
		return s == EMOrdoAnimState::CombatEnter
			|| s == EMOrdoAnimState::CombatLoop
			|| s == EMOrdoAnimState::CombatExit;
	};

	if (GetSprite())
	{
		if (IsCombatAnim(NewState) && !IsCombatAnim(OldState))
		{
			FVector scale = GetSprite()->GetRelativeScale3D();
			const float signX = (scale.X < 0.f) ? -1.f : 1.f;
			FVector newScale(signX * FMath::Abs(DefaultSpriteScale.X) * CombatSpriteScale,
				DefaultSpriteScale.Y * CombatSpriteScale,
				DefaultSpriteScale.Z * CombatSpriteScale);
			GetSprite()->SetRelativeScale3D(newScale);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue,
				FString::Printf(TEXT("[Ordo] 스케일 전투진입: Default(%.2f,%.2f,%.2f) * Combat(%.2f) → (%.2f,%.2f,%.2f)"),
				DefaultSpriteScale.X, DefaultSpriteScale.Y, DefaultSpriteScale.Z,
				CombatSpriteScale, newScale.X, newScale.Y, newScale.Z));
		}
		else if (!IsCombatAnim(NewState) && IsCombatAnim(OldState))
		{
			FVector scale = GetSprite()->GetRelativeScale3D();
			const float signX = (scale.X < 0.f) ? -1.f : 1.f;
			FVector newScale(signX * FMath::Abs(DefaultSpriteScale.X),
				DefaultSpriteScale.Y,
				DefaultSpriteScale.Z);
			GetSprite()->SetRelativeScale3D(newScale);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue,
				FString::Printf(TEXT("[Ordo] 스케일 전투이탈: 복원 → (%.2f,%.2f,%.2f)"),
				newScale.X, newScale.Y, newScale.Z));
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[Ordo] %s anim state → %d"), *OrdoName, static_cast<int32>(NewState));
}

void AMOrdoCharacter::PlayCombatExitAnim()
{
	CurrentAnimState = EMOrdoAnimState::CombatExit;

	// CombatEnter 플립북을 찾아 역재생
	const TSoftObjectPtr<UPaperFlipbook>* FoundFlipbook = AnimFlipbookMap.Find(EMOrdoAnimState::CombatEnter);
	if (FoundFlipbook && !FoundFlipbook->IsNull())
	{
		UPaperFlipbook* loadedFlipbook = FoundFlipbook->LoadSynchronous();
		if (loadedFlipbook && GetSprite())
		{
			GetSprite()->SetFlipbook(loadedFlipbook);
			// ReverseFromEnd()가 내부적으로 PlayRate를 반전(-1)하고 끝에서 재생
			GetSprite()->SetPlayRate(1.0f); // 양수 보장
			GetSprite()->ReverseFromEnd();  // → 내부에서 -1로 전환 후 끝에서 역재생
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[Ordo] %s CombatExit (reverse CombatEnter)"), *OrdoName);
}

void AMOrdoCharacter::FaceTarget(const AActor* Target)
{
	if (!Target || !GetSprite()) return;

	const float dirX = Target->GetActorLocation().X - GetActorLocation().X;
	FVector currentScale = GetSprite()->GetRelativeScale3D();

	// 화면상: 음수 X = 오른쪽 바라봄, 양수 X = 왼쪽 바라봄
	// 타겟이 오른쪽(dirX>0) → 음수, 타겟이 왼쪽(dirX<0) → 양수
	const float desiredX = (dirX >= 0.f) ? -FMath::Abs(currentScale.X) : FMath::Abs(currentScale.X);
	if (!FMath::IsNearlyEqual(currentScale.X, desiredX))
	{
		currentScale.X = desiredX;
		GetSprite()->SetRelativeScale3D(currentScale);
	}
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
