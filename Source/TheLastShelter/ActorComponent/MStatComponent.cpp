// Copyright TheLastShelter. All Rights Reserved.

#include "MStatComponent.h"

UMStatComponent::UMStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMStatComponent::BeginPlay()
{
	Super::BeginPlay();

	// BaseStat.Health 를 초기 체력으로 사용
	CurrentHealth = BaseStat.Health;
}

// ============================================================
// 초기화
// ============================================================

void UMStatComponent::InitializeFromPhysicalStat(const FMPhysicalStat& InStat)
{
	BaseStat = InStat;
	CurrentHealth = GetMaxHealth();
	OnHealthChanged.Broadcast(CurrentHealth);
}

// ============================================================
// 스탯 접근
// ============================================================

FMPhysicalStat UMStatComponent::GetEffectiveStat() const
{
	FMPhysicalStat effective;
	effective.Attack      = BaseStat.Attack      + BonusStat.Attack;
	effective.Defense     = BaseStat.Defense     + BonusStat.Defense;
	effective.Health      = BaseStat.Health      + BonusStat.Health;
	effective.Stamina     = BaseStat.Stamina     + BonusStat.Stamina;
	effective.MoveSpeed   = BaseStat.MoveSpeed   + BonusStat.MoveSpeed;
	effective.WorkSpeed   = BaseStat.WorkSpeed   + BonusStat.WorkSpeed;
	effective.AttackSpeed = BaseStat.AttackSpeed + BonusStat.AttackSpeed;
	effective.Recovery    = BaseStat.Recovery    + BonusStat.Recovery;
	effective.Accuracy    = BaseStat.Accuracy    + BonusStat.Accuracy;
	effective.Evasion     = BaseStat.Evasion     + BonusStat.Evasion;
	effective.CritChance  = BaseStat.CritChance  + BonusStat.CritChance;
	effective.CritDamage  = BaseStat.CritDamage  + BonusStat.CritDamage;
	return effective;
}

void UMStatComponent::SetBonusStat(const FMPhysicalStat& InBonus)
{
	BonusStat = InBonus;
}

// ============================================================
// 체력
// ============================================================

float UMStatComponent::GetHealthPercent() const
{
	float maxHp = GetMaxHealth();
	if (maxHp <= 0.f) return 0.f;
	return FMath::Clamp(CurrentHealth / maxHp, 0.f, 1.f);
}

// ============================================================
// 데미지 & 회복
// ============================================================

float UMStatComponent::ApplyDamage(float RawDamage)
{
	if (IsDead()) return 0.f;

	FMPhysicalStat effectiveStat = GetEffectiveStat();
	float actualDamage = FMath::Max(RawDamage - effectiveStat.Defense, 0.f);

	CurrentHealth = FMath::Max(CurrentHealth - actualDamage, 0.f);
	OnHealthChanged.Broadcast(CurrentHealth);

	if (IsDead())
	{
		OnDeath.Broadcast();
	}

	return actualDamage;
}

void UMStatComponent::Heal(float Amount)
{
	if (IsDead()) return;

	CurrentHealth = FMath::Min(CurrentHealth + Amount, GetMaxHealth());
	OnHealthChanged.Broadcast(CurrentHealth);
}

void UMStatComponent::SetHealth(float NewHealth)
{
	CurrentHealth = FMath::Clamp(NewHealth, 0.f, GetMaxHealth());
	OnHealthChanged.Broadcast(CurrentHealth);

	if (IsDead())
	{
		OnDeath.Broadcast();
	}
}

// ============================================================
// 전투력 계산
// ============================================================

float UMStatComponent::CalculateCombatPower() const
{
	FMPhysicalStat effectiveStat = GetEffectiveStat();

	// 전투력 = 공격력*2 + 방어력*1.5 + 체력*0.5 + 크리확률*크리데미지*100
	float power = effectiveStat.Attack * 2.f
	            + effectiveStat.Defense * 1.5f
	            + effectiveStat.Health * 0.5f
	            + effectiveStat.CritChance * effectiveStat.CritDamage * 100.f;

	return power;
}
