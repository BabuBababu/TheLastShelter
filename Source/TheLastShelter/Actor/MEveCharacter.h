// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "MDataTypes.h"
#include "MEveCharacter.generated.h"

class UMDataManager;
class UPaperFlipbook;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * MEveCharacter
 * 미소녀(Eve) 캐릭터. 플레이어 편.
 * AI로 자율 행동하며, 감정/스탯/호감도를 가짐.
 * PaperZDCharacter의 내장 Sprite(GetSprite())로 상태별 스프라이트 전환.
 */
UCLASS()
class THELASTSHELTER_API AMEveCharacter : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AMEveCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// ---- 데이터 초기화 ----
	UFUNCTION(BlueprintCallable, Category = "Eve")
	void InitializeFromData(const FString& EveID);

	// ============================================================
	// 컴포넌트
	// ============================================================

	/** 공격 이펙트 (나이아가라) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eve|VFX")
	TObjectPtr<UNiagaraComponent> AttackVFXComp;

	/** 피격 이펙트 (나이아가라) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Eve|VFX")
	TObjectPtr<UNiagaraComponent> HitVFXComp;

	// ============================================================
	// 애니메이션 상태 & 플립북 매핑
	// ============================================================

	/** 현재 애니메이션 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Animation")
	EMEveAnimState CurrentAnimState = EMEveAnimState::Idle;

	/** 애니메이션 상태 → 플립북 에셋 매핑 테이블 (에디터에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Animation")
	TMap<EMEveAnimState, TSoftObjectPtr<UPaperFlipbook>> AnimFlipbookMap;

	/** 상태 변경 → 플립북 교체 */
	UFUNCTION(BlueprintCallable, Category = "Eve|Animation")
	void SetAnimState(EMEveAnimState NewState);

	/** 무장 여부 (true=총 장착 → Gun* 애니메이션 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Animation")
	bool IsArmed = false;

	/** 이동 속도 기반 자동 상태 전환 (Tick에서 매 프레임 호출) */
	UFUNCTION(BlueprintCallable, Category = "Eve|Animation")
	void UpdateAnimStateFromMovement();

	/** 이 속도 이상이면 Run 애니메이션으로 전환 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Animation")
	float RunSpeedThreshold = 350.f;

	/** 액션 애니메이션(공격/피격) 재생 중 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "Eve|Animation")
	bool IsPlayingActionAnim = false;

	float ActionAnimEndTime = 0.f;

	/** 공격/피격 애니메이션 재생 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Animation")
	float ActionAnimDuration = 0.5f;

	// ============================================================
	// VFX 시스템
	// ============================================================

	/** 공격 VFX 에셋 (에디터에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Eve|VFX")
	TObjectPtr<UNiagaraSystem> AttackVFXAsset;

	/** 피격 VFX 에셋 (에디터에서 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Eve|VFX")
	TObjectPtr<UNiagaraSystem> HitVFXAsset;

	/** 공격 이펙트 재생 */
	UFUNCTION(BlueprintCallable, Category = "Eve|VFX")
	void PlayAttackVFX();

	/** 피격 이펙트 재생 */
	UFUNCTION(BlueprintCallable, Category = "Eve|VFX")
	void PlayHitVFX();

	// ============================================================
	// Eve 고유 데이터
	// ============================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Data")
	FString EveDataID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Data")
	FString EveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Stat")
	FMPhysicalStat PhysicalStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Stat")
	FMMentalStat MentalStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Stat")
	float Affection = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Stat")
	TArray<EMHiddenStatType> HiddenStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Stat")
	TArray<FString> SkillIDs;

	// ---- 감정 ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eve|Emotion")
	EMEmotion CurrentEmotion = EMEmotion::Normal;

	UFUNCTION(BlueprintCallable, Category = "Eve|Emotion")
	void SetEmotion(EMEmotion NewEmotion);

	// ---- 히든 스탯 ----
	UFUNCTION(BlueprintCallable, Category = "Eve|HiddenStat")
	void AssignRandomHiddenStats();

	// ---- 전투 ----
	UPROPERTY(BlueprintReadOnly, Category = "Eve|Combat")
	bool IsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Eve|Combat")
	float CurrentHealth = 100.f;

	/** AI가 호출하는 공격 함수 */
	UFUNCTION(BlueprintCallable, Category = "Eve|Combat")
	void PerformAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Eve|Combat")
	void TakeDamageFromOrdo(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Eve|Combat")
	bool IsDead() const { return CurrentHealth <= 0.f; }
};
