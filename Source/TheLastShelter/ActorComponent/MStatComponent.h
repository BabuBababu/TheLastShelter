// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MDataTypes.h"
#include "MStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

/**
 * MStatComponent
 * Player, Eve, Ordo가 공통으로 부착하는 범용 스탯 컴포넌트.
 * 체력/공격력/이속 등 전투 관련 수치를 통합 관리.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class THELASTSHELTER_API UMStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMStatComponent();

	// ============================================================
	// 초기화
	// ============================================================

	/** FMPhysicalStat 으로 스탯 일괄 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void InitializeFromPhysicalStat(const FMPhysicalStat& InStat);

	// ============================================================
	// 스탯 접근
	// ============================================================

	UFUNCTION(BlueprintPure, Category = "Stat")
	const FMPhysicalStat& GetBaseStat() const { return BaseStat; }

	/** 버프/장비 보정이 적용된 최종 스탯 */
	UFUNCTION(BlueprintPure, Category = "Stat")
	FMPhysicalStat GetEffectiveStat() const;

	/** 버프/장비에 의한 추가 스탯 설정 */
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void SetBonusStat(const FMPhysicalStat& InBonus);

	UFUNCTION(BlueprintPure, Category = "Stat")
	const FMPhysicalStat& GetBonusStat() const { return BonusStat; }

	// ============================================================
	// 체력
	// ============================================================

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	float GetMaxHealth() const { return GetEffectiveStat().Health; }

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	bool IsDead() const { return CurrentHealth <= 0.f; }

	UFUNCTION(BlueprintPure, Category = "Stat|Health")
	float GetHealthPercent() const;

	// ============================================================
	// 데미지 & 회복
	// ============================================================

	/**
	 * 데미지 적용. Defense 반영하여 실제 피해량 계산.
	 * @return 실제 적용된 데미지
	 */
	UFUNCTION(BlueprintCallable, Category = "Stat|Combat")
	float ApplyDamage(float RawDamage);

	/** 체력 회복 */
	UFUNCTION(BlueprintCallable, Category = "Stat|Health")
	void Heal(float Amount);

	/** 체력을 직접 설정 (부활 등) */
	UFUNCTION(BlueprintCallable, Category = "Stat|Health")
	void SetHealth(float NewHealth);

	// ============================================================
	// 전투력 계산
	// ============================================================

	/** 전투력 수치 산출 (기지 가치 계산용) */
	UFUNCTION(BlueprintPure, Category = "Stat|Combat")
	float CalculateCombatPower() const;

	// ============================================================
	// 이동속도 헬퍼
	// ============================================================

	UFUNCTION(BlueprintPure, Category = "Stat")
	float GetMoveSpeed() const { return GetEffectiveStat().MoveSpeed; }

	UFUNCTION(BlueprintPure, Category = "Stat")
	float GetAttackPower() const { return GetEffectiveStat().Attack; }

	// ============================================================
	// 이벤트
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "Stat|Event")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stat|Event")
	FOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FMPhysicalStat BaseStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FMPhysicalStat BonusStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat|Health")
	float CurrentHealth = 100.f;
};
