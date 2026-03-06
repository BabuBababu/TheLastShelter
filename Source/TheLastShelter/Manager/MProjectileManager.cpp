// Copyright TheLastShelter. All Rights Reserved.

#include "Manager/MProjectileManager.h"
#include "Actor/MBulletBase.h"
#include "Actor/MEveCharacter.h"
#include "Actor/MOrdoCharacter.h"
#include "Actor/MPlayerCharacter.h"
#include "ActorComponent/MStatComponent.h"
#include "Manager/MLogManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
// Subsystem Lifecycle
// ============================================================

void UMProjectileManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("[ProjectileManager] Initialized"));
}

void UMProjectileManager::Deinitialize()
{
	// 클래스별 풀 전체 정리 — 월드가 이미 정리 중이면 액터는 자동 파괴됨
	for (auto& Pair : BulletPools)
	{
		for (AMBulletBase* bullet : Pair.Value)
		{
			if (IsValid(bullet))
			{
				bullet->Destroy();
			}
		}
	}
	BulletPools.Empty();
	UE_LOG(LogTemp, Log, TEXT("[ProjectileManager] Deinitialized"));
	Super::Deinitialize();
}

// ============================================================
// Pool Management
// ============================================================

void UMProjectileManager::EnsurePool(UWorld* World, TSubclassOf<AMBulletBase> BulletClass)
{
	if (!World || !BulletClass) return;

	// 이미 해당 클래스 풀이 있으면 스킵
	if (BulletPools.Contains(BulletClass)) return;

	TArray<AMBulletBase*>& pool = BulletPools.Add(BulletClass);
	pool.Reserve(PoolSizePerClass);

	FActorSpawnParameters spawnParams;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < PoolSizePerClass; ++i)
	{
		AMBulletBase* bullet = World->SpawnActor<AMBulletBase>(
			BulletClass, FVector(0.f, 0.f, -10000.f), FRotator::ZeroRotator, spawnParams);
		if (bullet)
		{
			bullet->Deactivate();
			pool.Add(bullet);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[ProjectileManager] Pool created — class=%s count=%d"),
		*BulletClass->GetName(), pool.Num());
}

AMBulletBase* UMProjectileManager::GetBullet(TSubclassOf<AMBulletBase> BulletClass)
{
	if (!BulletClass) return nullptr;

	TArray<AMBulletBase*>* pool = BulletPools.Find(BulletClass);
	if (!pool) return nullptr;

	for (AMBulletBase* bullet : *pool)
	{
		if (bullet && !bullet->IsPoolActive())
		{
			return bullet;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[ProjectileManager] Pool exhausted — class=%s"),
		*BulletClass->GetName());
	return nullptr;
}

// ============================================================
// FireBullet — 원스탑 발사 API
// ============================================================

void UMProjectileManager::FireBullet(AActor* Instigator, AActor* Target, FVector MuzzleOffset)
{
	if (!Instigator || !Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ProjectileManager] FireBullet — Instigator or Target is null!"));
		return;
	}

	// 캐릭터의 BulletClass 추출
	TSubclassOf<AMBulletBase> bulletClass = GetBulletClass(Instigator);
	if (!bulletClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ProjectileManager] FireBullet — BulletClass is NULL for %s! BP에서 BulletClass를 설정하세요."), *Instigator->GetName());
		return;
	}

	// 풀이 없으면 즉시 생성
	EnsurePool(Instigator->GetWorld(), bulletClass);

	AMBulletBase* bullet = GetBullet(bulletClass);
	if (!bullet) return;

	// 병과 추출
	EMWeaponClass weaponClass = GetWeaponClass(Instigator);

	// 대미지 & 명중률 계산
	float damage   = CalculateDamage(Instigator);
	float accuracy = CalculateAccuracy(Instigator);

	// 발사 위치
	FVector startLocation = Instigator->GetActorLocation() + MuzzleOffset;

	// 타겟 위치 (오차 적용)
	FVector targetLocation = ApplyAccuracyOffset(Target->GetActorLocation(), accuracy);

	bullet->Fire(startLocation, targetLocation, damage, Instigator, weaponClass);

	UE_LOG(LogTemp, Log, TEXT("[ProjectileManager] %s (%d) → %s | dmg=%.1f acc=%.0f%%"),
		*Instigator->GetName(), (int32)weaponClass, *Target->GetName(), damage, accuracy);

	// CombatLog
	if (UMLogManager* logMgr = GetGameInstance()->GetSubsystem<UMLogManager>())
	{
		logMgr->Logf(TEXT("Projectile"), TEXT("FireBullet: %s → %s | bullet=%s dmg=%.1f acc=%.0f%% weapon=%d"),
			*UMLogManager::ActorID(Instigator),
			*UMLogManager::ActorID(Target),
			*UMLogManager::ActorID(bullet),
			damage, accuracy, (int32)weaponClass);
	}
}

// ============================================================
// 대미지 계산
// ============================================================

float UMProjectileManager::CalculateDamage(AActor* Instigator)
{
	if (!Instigator) return 0.f;

	// ---- Eve ----
	if (AMEveCharacter* eve = Cast<AMEveCharacter>(Instigator))
	{
		float attack  = eve->StatComp ? eve->StatComp->GetAttackPower() : 0.f;
		float combat  = FMath::Max(eve->MentalStat.Combat, 1.f);
		float hidden  = GetHiddenDamageBonus(eve->HiddenStats);

		return attack * combat + hidden;
	}

	// ---- Ordo ----
	if (AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(Instigator))
	{
		float attack = ordo->StatComp ? ordo->StatComp->GetAttackPower() : 0.f;
		return attack; // MentalStat 없음
	}

	return 0.f;
}

// ============================================================
// 명중률 계산
// ============================================================

float UMProjectileManager::CalculateAccuracy(AActor* Instigator)
{
	if (!Instigator) return 60.f;

	constexpr float baseAccuracy = 60.f;

	// ---- Eve ----
	if (AMEveCharacter* eve = Cast<AMEveCharacter>(Instigator))
	{
		float combat = eve->MentalStat.Combat;
		float hidden = GetHiddenAccuracyBonus(eve->HiddenStats);

		return baseAccuracy + combat * 0.2f + hidden;
	}

	// ---- Ordo ----
	if (AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(Instigator))
	{
		float physAccuracy = ordo->StatComp
			? ordo->StatComp->GetEffectiveStat().Accuracy
			: 0.f;
		return baseAccuracy + physAccuracy;
	}

	return baseAccuracy;
}

// ============================================================
// 명중률 → 오차 적용
// ============================================================

FVector UMProjectileManager::ApplyAccuracyOffset(const FVector& TargetLocation, float Accuracy)
{
	// 100% 이상이면 정확한 위치
	if (Accuracy >= 100.f)
	{
		return TargetLocation;
	}

	// 오차 범위 (단위: 언리얼 유닛)
	float errorRange = FMath::Max(100.f - Accuracy, 0.f);

	// 2D 게임이므로 X, Z 축에만 오프셋 적용 (Y는 깊이)
	float offsetX = FMath::RandRange(-errorRange, errorRange);
	float offsetZ = FMath::RandRange(-errorRange, errorRange);

	return TargetLocation + FVector(offsetX, 0.f, offsetZ);
}

// ============================================================
// HiddenStat 보너스
// ============================================================

float UMProjectileManager::GetHiddenDamageBonus(const TArray<EMHiddenStatType>& HiddenStats)
{
	float bonus = 0.f;

	for (EMHiddenStatType stat : HiddenStats)
	{
		switch (stat)
		{
		case EMHiddenStatType::StrongWill:
			bonus += 5.f;  // 강한의지 → 대미지 +5
			break;
		case EMHiddenStatType::Sharpshooter:
			bonus += 3.f;  // 백발백중 → 대미지 +3 (주된 효과는 명중률)
			break;
		default:
			break;
		}
	}

	return bonus;
}

float UMProjectileManager::GetHiddenAccuracyBonus(const TArray<EMHiddenStatType>& HiddenStats)
{
	float bonus = 0.f;

	for (EMHiddenStatType stat : HiddenStats)
	{
		switch (stat)
		{
		case EMHiddenStatType::Sharpshooter:
			bonus += 30.f;  // 백발백중 → 명중률 +30
			break;
		case EMHiddenStatType::Calm:
			bonus += 10.f;  // 침착함 → 명중률 +10
			break;
		case EMHiddenStatType::Swiftness:
			bonus += 5.f;   // 신속함 → 명중률 +5
			break;
		default:
			break;
		}
	}

	return bonus;
}

float UMProjectileManager::GetHiddenAttackSpeedBonus(const TArray<EMHiddenStatType>& HiddenStats)
{
	float bonus = 0.f;

	for (EMHiddenStatType stat : HiddenStats)
	{
		switch (stat)
		{
		case EMHiddenStatType::Swiftness:
			bonus += 1.f;  // 신속함 → 공격속도 +1발/초
			break;
		case EMHiddenStatType::Agile:
			bonus += 2.f;  // 재빠름 → 공격속도 +2발/초
			break;
		default:
			break;
		}
	}

	return bonus;
}

// ============================================================
// 병과 추출
// ============================================================

EMWeaponClass UMProjectileManager::GetWeaponClass(AActor* Instigator)
{
	if (!Instigator) return EMWeaponClass::HG;

	if (AMEveCharacter* eve = Cast<AMEveCharacter>(Instigator))
		return eve->WeaponClass;

	if (AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(Instigator))
		return ordo->WeaponClass;

	if (AMPlayerCharacter* player = Cast<AMPlayerCharacter>(Instigator))
		return player->WeaponClass;

	return EMWeaponClass::HG;
}

// ============================================================
// BulletClass 추출
// ============================================================

TSubclassOf<AMBulletBase> UMProjectileManager::GetBulletClass(AActor* Instigator)
{
	if (!Instigator) return nullptr;

	if (AMEveCharacter* eve = Cast<AMEveCharacter>(Instigator))
		return eve->BulletClass;

	if (AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(Instigator))
		return ordo->BulletClass;

	if (AMPlayerCharacter* player = Cast<AMPlayerCharacter>(Instigator))
		return player->BulletClass;

	return nullptr;
}
