// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MProjectileManager.generated.h"

class AMBulletBase;

/**
 * MProjectileManager — 입자 클래스별 오브젝트 풀 + 대미지/명중률 계산.
 *
 * ■ 캐릭터(Eve/Ordo/Player)마다 BulletClass 프로퍼티를 지정
 * ■ 클래스별 풀 자동 생성 (BP_Bullet_AR, BP_Bullet_SR 각각 분리)
 * ■ FireBullet — 캐릭터의 BulletClass 풀에서 총알을 꺼내 발사
 */
UCLASS()
class THELASTSHELTER_API UMProjectileManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ============================================================
	// Subsystem Lifecycle
	// ============================================================
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================
	// 풀 관리
	// ============================================================

	/**
	 * 해당 클래스의 풀이 없으면 생성.
	 * 캐릭터 BeginPlay 또는 첫 FireBullet 시 자동 호출.
	 */
	void EnsurePool(UWorld* World, TSubclassOf<AMBulletBase> BulletClass);

	/** 해당 클래스 풀에서 비활성 총알 1개 대여 */
	AMBulletBase* GetBullet(TSubclassOf<AMBulletBase> BulletClass);

	// ============================================================
	// 원스탑 발사 API
	// ============================================================

	/**
	 * 캐릭터의 BulletClass 풀에서 총알을 꺼내 대미지/명중률 계산 후 발사.
	 * @param Instigator   발사자 (Eve / Ordo / Player)
	 * @param Target       타겟 액터
	 * @param MuzzleOffset 총구 오프셋 (옵션)
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void FireBullet(AActor* Instigator, AActor* Target,
	                FVector MuzzleOffset = FVector::ZeroVector);

	// ============================================================
	// 대미지 / 명중률 계산
	// ============================================================

	UFUNCTION(BlueprintPure, Category = "Projectile|Damage")
	static float CalculateDamage(AActor* Instigator);

	UFUNCTION(BlueprintPure, Category = "Projectile|Accuracy")
	static float CalculateAccuracy(AActor* Instigator);

	UFUNCTION(BlueprintPure, Category = "Projectile|Accuracy")
	static FVector ApplyAccuracyOffset(const FVector& TargetLocation, float Accuracy);

	// ============================================================
	// 캐릭터에서 정보 추출
	// ============================================================

	/** 캐릭터의 TSubclassOf<AMBulletBase> BulletClass 추출 */
	UFUNCTION(BlueprintPure, Category = "Projectile")
	static TSubclassOf<AMBulletBase> GetBulletClass(AActor* Instigator);

	/** 캐릭터의 EMWeaponClass 추출 */
	UFUNCTION(BlueprintPure, Category = "Projectile")
	static EMWeaponClass GetWeaponClass(AActor* Instigator);

	// ============================================================
	// HiddenStat 보너스
	// ============================================================

	static float GetHiddenDamageBonus(const TArray<EMHiddenStatType>& HiddenStats);
	static float GetHiddenAccuracyBonus(const TArray<EMHiddenStatType>& HiddenStats);

	/** 히든스탯에서 AttackSpeed(발/초) 보너스 합산.
	 *  Swiftness +1, Agile +2 */
	static float GetHiddenAttackSpeedBonus(const TArray<EMHiddenStatType>& HiddenStats);

	// ============================================================
	// 설정
	// ============================================================

	/** 클래스당 풀 크기 (기본 20) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Pool")
	int32 PoolSizePerClass = 20;

private:
	/** 클래스별 오브젝트 풀 (월드 소유 액터 → GC 자동 관리) */
	TMap<TSubclassOf<AMBulletBase>, TArray<AMBulletBase*>> BulletPools;
};
