// Copyright TheLastShelter. All Rights Reserved.

#include "MEveCharacter.h"
#include "MStatComponent.h"
#include "MDataManager.h"
#include "MInventoryManager.h"
#include "MProjectileManager.h"
#include "MOrdoCharacter.h"
#include "MEveAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/Engine.h"

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

void AMEveCharacter::BeginPlay()
{
	Super::BeginPlay();

	// EveDataId가 설정되어 있으면 데이터에서 초기화
	if (EveDataId > 0)
	{
		InitializeFromData(EveDataId);
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

	// 초기 애니메이션 상태 강제 적용 (Idle 플립북 표시)
	const EMEveAnimState InitState = IsArmed ? EMEveAnimState::GunIdle : EMEveAnimState::Idle;
	CurrentAnimState = EMEveAnimState::MAX; // 강제 갱신을 위해 다른 값으로 초기화
	SetAnimState(InitState);

	// 스프라이트 기본 스케일 저장 (전투 스케일 복원에 사용)
	if (GetSprite())
	{
		DefaultSpriteScale = GetSprite()->GetRelativeScale3D();
	}
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

void AMEveCharacter::InitializeFromData(int32 EveId)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return;

	UMDataManager* DataMgr = GI->GetSubsystem<UMDataManager>();
	if (!DataMgr) return;

	FMEveData Data;
	if (DataMgr->GetEveDataByID(EveId, Data))
	{
		EveDataId = Data.Id;
		EveName = Data.Name;
		MentalStat = Data.MentalStat;
		Affection = Data.Affection;
		HiddenStats = Data.HiddenStats;
		SkillIDs = Data.SkillIDs;

		if (StatComp)
		{
			StatComp->InitializeFromPhysicalStat(Data.PhysicalStat);
		}

		// MoveSpeed 적용
		GetCharacterMovement()->MaxWalkSpeed = Data.PhysicalStat.MoveSpeed;

		// 인벤토리 매니저에 이 Eve의 인벤토리 생성
		const FString ownerKey = FString::FromInt(EveDataId);
		UMInventoryManager* InvMgr = GI->GetSubsystem<UMInventoryManager>();
		if (InvMgr && !InvMgr->HasInventory(ownerKey))
		{
			InvMgr->CreateInventory(ownerKey, 20);
		}

		UE_LOG(LogTemp, Log, TEXT("[Eve] Initialized: %s (Id=%d)"), *EveName, EveDataId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Eve] Failed to load data for Id: %d"), EveId);
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

void AMEveCharacter::TakeDamageFromOrdo(float Damage, AActor* Attacker)
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

	UE_LOG(LogTemp, Log, TEXT("[Eve] %s took %.1f damage (%.1f HP left)"),
		*EveName, actualDamage, StatComp->GetCurrentHealth());

	if (StatComp->IsDead())
	{
		SetAnimState(EMEveAnimState::Down);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 99999.f;
		UE_LOG(LogTemp, Log, TEXT("[Eve] %s is dead!"), *EveName);
	}
	else
	{
		SetAnimState(EMEveAnimState::Hit);
		IsPlayingActionAnim = true;
		ActionAnimEndTime = GetWorld()->GetTimeSeconds() + 0.3f;
	}
}

bool AMEveCharacter::IsDead() const
{
	return StatComp ? StatComp->IsDead() : true;
}

// ============================================================
// 공격
// ============================================================

void AMEveCharacter::PerformAttack(AActor* Target)
{
	UE_LOG(LogTemp, Log, TEXT("[Eve] PerformAttack called — Dead=%d Target=%s ActionAnim=%d"),
		IsDead(), Target ? *Target->GetName() : TEXT("null"), IsPlayingActionAnim);

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
			// 총구 오프셋: 캐릭터 전방 약간 위
			FVector muzzleOffset = FVector(0.f, 0.f, 10.f);
			projMgr->FireBullet(this, Target, muzzleOffset);
		}
	}
}

// ============================================================
// 이동 기반 상태 자동 전환
// ============================================================

void AMEveCharacter::UpdateAnimStateFromMovement()
{
	if (IsPlayingActionAnim || IsDead()) return;

	// CombatEnter / CombatLoop / CombatExit — AI가 직접 관리하므로 항상 보호
	if (CurrentAnimState == EMEveAnimState::CombatEnter ||
		CurrentAnimState == EMEveAnimState::CombatLoop ||
		CurrentAnimState == EMEveAnimState::CombatExit)
	{
		return;
	}

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

	const EMEveAnimState OldState = CurrentAnimState;
	CurrentAnimState = NewState;

	// 매핑 테이블에서 해당 상태의 플립북 에셋을 찾아 교체
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

			// ★ 플립북 실제 재생 로그 (이 로그가 뜨면 PlayFromStart가 호출된 것)
			if (GEngine)
			{
				const FString name = StaticEnum<EMEveAnimState>()->GetNameStringByValue(static_cast<int64>(NewState));
				const uint64 frame = GFrameCounter;
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
					FString::Printf(TEXT("[Eve] ★ 플립북 재생: %s (Frame %llu)"), *name, frame));
			}
		}
	}

	// --- 방향별 스프라이트 좌우 반전 ---
	// 화면상 양수 X = 왼쪽 바라봄, 음수 X = 오른쪽 바라봄
	// 플립북 기본형(에디터)은 오른쪽을 바라보지만, 렌더링 시 X축 반전됨
	//   *_Right → X음수(화면 오른쪽 바라봄), *_Left → X양수(화면 왼쪽 바라봄)
	auto IsRightFacing = [](EMEveAnimState s)
	{
		return s == EMEveAnimState::Walk_Right || s == EMEveAnimState::GunWalk_Right ||
		       s == EMEveAnimState::Run_Right  || s == EMEveAnimState::GunRun_Right ||
		       s == EMEveAnimState::BoxCarry_Right;
	};
	auto IsLeftFacing = [](EMEveAnimState s)
	{
		return s == EMEveAnimState::Walk_Left || s == EMEveAnimState::GunWalk_Left ||
		       s == EMEveAnimState::Run_Left  || s == EMEveAnimState::GunRun_Left ||
		       s == EMEveAnimState::BoxCarry_Left;
	};

	if (GetSprite())
	{
		FVector scale = GetSprite()->GetRelativeScale3D();
		if (IsRightFacing(NewState))
		{
			scale.X = -FMath::Abs(scale.X); // 오른쪽 바라봄 → 음수
			GetSprite()->SetRelativeScale3D(scale);
		}
		else if (IsLeftFacing(NewState))
		{
			scale.X = FMath::Abs(scale.X); // 왼쪽 바라봄 → 양수 (좌우반전)
			GetSprite()->SetRelativeScale3D(scale);
		}
	}

	// --- 전투 스프라이트 스케일 적용/복원 ---
	auto IsCombatAnim = [](EMEveAnimState s)
	{
		return s == EMEveAnimState::CombatEnter
			|| s == EMEveAnimState::CombatLoop
			|| s == EMEveAnimState::CombatExit;
	};

	if (GetSprite())
	{
		if (IsCombatAnim(NewState) && !IsCombatAnim(OldState))
		{
			// 전투 진입 → DefaultSpriteScale * CombatSpriteScale 적용 (FaceTarget X flip 부호 보존)
			FVector scale = GetSprite()->GetRelativeScale3D();
			const float signX = (scale.X < 0.f) ? -1.f : 1.f;
			FVector newScale(signX * FMath::Abs(DefaultSpriteScale.X) * CombatSpriteScale,
				DefaultSpriteScale.Y * CombatSpriteScale,
				DefaultSpriteScale.Z * CombatSpriteScale);
			GetSprite()->SetRelativeScale3D(newScale);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue,
				FString::Printf(TEXT("[Eve] 스케일 전투진입: signX=%.0f, Default(%.2f) * Combat(%.2f) → X=%.2f"),
				signX, DefaultSpriteScale.X, CombatSpriteScale, newScale.X));
		}
		else if (!IsCombatAnim(NewState) && IsCombatAnim(OldState))
		{
			// 전투 이탈 → 기본 스케일 복원
			FVector scale = GetSprite()->GetRelativeScale3D();
			const float signX = (scale.X < 0.f) ? -1.f : 1.f;
			FVector newScale(signX * FMath::Abs(DefaultSpriteScale.X),
				DefaultSpriteScale.Y,
				DefaultSpriteScale.Z);
			GetSprite()->SetRelativeScale3D(newScale);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue,
				FString::Printf(TEXT("[Eve] 스케일 전투이탈: signX=%.0f → X=%.2f"),
				signX, newScale.X));
		}
	}

	// ---- 플립북 전환 상세 디버그 (X 스케일 + 방향) ----
	if (GEngine)
	{
		const FString stateName = StaticEnum<EMEveAnimState>()->GetNameStringByValue(static_cast<int64>(NewState));
		const FString oldStateName = StaticEnum<EMEveAnimState>()->GetNameStringByValue(static_cast<int64>(OldState));
		const float scaleX = GetSprite() ? GetSprite()->GetRelativeScale3D().X : 0.f;
		const TCHAR* facingDir = (scaleX <= 0.f) ? TEXT("→우") : TEXT("←좌");

		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White,
			FString::Printf(TEXT("[Eve] 상태: %s → %s (X=%.2f %s)"), *oldStateName, *stateName, scaleX, facingDir));
	}
}

void AMEveCharacter::PlayCombatExitAnim()
{
	CurrentAnimState = EMEveAnimState::CombatExit;

	// CombatEnter 플립북을 찾아 역재생
	const TSoftObjectPtr<UPaperFlipbook>* FoundFlipbook = AnimFlipbookMap.Find(EMEveAnimState::CombatEnter);
	if (FoundFlipbook && !FoundFlipbook->IsNull())
	{
		UPaperFlipbook* loadedFlipbook = FoundFlipbook->LoadSynchronous();
		if (loadedFlipbook && GetSprite())
		{
			GetSprite()->SetFlipbook(loadedFlipbook);
			// ReverseFromEnd()가 내부적으로 PlayRate를 반전(-1)하고 끝에서 재생
			// 주의: SetPlayRate(-1) 을 먼저 호출하면 ReverseFromEnd가 다시 부정해 +1이 됨
			GetSprite()->SetPlayRate(1.0f); // 양수 보장
			GetSprite()->ReverseFromEnd();  // → 내부에서 -1로 전환 후 끝에서 역재생
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("[Eve] %s CombatExit (reverse CombatEnter)"), *EveName);
}

void AMEveCharacter::FaceTarget(const AActor* Target)
{
	if (!Target || !GetSprite()) return;

	const float dirX = Target->GetActorLocation().X - GetActorLocation().X;
	FVector currentScale = GetSprite()->GetRelativeScale3D();

	// 화면상: 음수 X = 오른쪽 바라봄, 양수 X = 왼쪽 바라봄
	// 타겟이 오른쪽(dirX>0) → 음수 (오른쪽 바라봄)
	// 타겟이 왼쪽(dirX<0) → 양수 (왼쪽 바라봄)
	const float desiredX = (dirX >= 0.f) ? -FMath::Abs(currentScale.X) : FMath::Abs(currentScale.X);
	if (!FMath::IsNearlyEqual(currentScale.X, desiredX))
	{
		currentScale.X = desiredX;
		GetSprite()->SetRelativeScale3D(currentScale);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple,
			FString::Printf(TEXT("[Eve] FaceTarget: dirX=%.0f → X=%.2f (%s)"),
				dirX, desiredX, desiredX <= 0.f ? TEXT("→우") : TEXT("←좌")));
	}
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
