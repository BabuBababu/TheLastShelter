// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "MDataTypes.h"
#include "Actor/MBulletBase.h"
#include "MOrdoCharacter.generated.h"

class UMDataManager;
class UMStatComponent;
class UPaperFlipbook;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * MOrdoCharacter
 * 적 캐릭터 베이스. 엑시움/신테아 공통.
 * 물리 스탯만 보유, AI로 제어.
 * PaperZDCharacter의 내장 Sprite(GetSprite())로 상태별 스프라이트 전환.
 */
UCLASS()
class THELASTSHELTER_API AMOrdoCharacter : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AMOrdoCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Ordo")
	void InitializeFromData(int32 OrdoId);

	// ============================================================
	// 컴포넌트
	// ============================================================

	/** 스탯 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ordo|Stat")
	TObjectPtr<UMStatComponent> StatComp;

	/** 공격 이펙트 (나이아가라) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ordo|VFX")
	TObjectPtr<UNiagaraComponent> AttackVFXComp;

	/** 피격 이펙트 (나이아가라) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ordo|VFX")
	TObjectPtr<UNiagaraComponent> HitVFXComp;

	// ============================================================
	// 애니메이션 상태 & 플립북 매핑
	// ============================================================

	/** 현재 애니메이션 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Animation")
	EMOrdoAnimState CurrentAnimState = EMOrdoAnimState::GunIdle;

	/** 애니메이션 상태 → 플립북 에셋 매핑 테이블 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Animation")
	TMap<EMOrdoAnimState, TSoftObjectPtr<UPaperFlipbook>> AnimFlipbookMap;

	/** 상태 변경 → 플립북 교체 */
	UFUNCTION(BlueprintCallable, Category = "Ordo|Animation")
	void SetAnimState(EMOrdoAnimState NewState);

	/** 전투 애니메이션(CombatEnter/CombatLoop/CombatExit) 스프라이트 스케일 배수 (1.0 = 원본, 0.5 = 절반) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Animation")
	float CombatSpriteScale = 1.0f;

	/** 스프라이트 기본 스케일 (BeginPlay에서 자동 저장, 전투 스케일 복원에 사용) */
	FVector DefaultSpriteScale = FVector::OneVector;

	/** CombatEnter 플립북을 역재생하여 CombatExit 상태로 전환 */
	UFUNCTION(BlueprintCallable, Category = "Ordo|Animation")
	void PlayCombatExitAnim();

	/** 타겟 방향으로 스프라이트 좌우 반전 (X축 기준) */
	UFUNCTION(BlueprintCallable, Category = "Ordo|Animation")
	void FaceTarget(const AActor* Target);

	/** 이동 속도 기반 자동 상태 전환 (Tick에서 매 프레임 호출) */
	UFUNCTION(BlueprintCallable, Category = "Ordo|Animation")
	void UpdateAnimStateFromMovement();

	/** 이 속도 이상이면 Run 애니메이션으로 전환 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Animation")
	float RunSpeedThreshold = 350.f;

	/** 액션 애니메이션(공격/피격) 재생 중 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "Ordo|Animation")
	bool IsPlayingActionAnim = false;

	float ActionAnimEndTime = 0.f;

	/** 공격/피격 애니메이션 재생 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Animation")
	float ActionAnimDuration = 0.5f;

	// ============================================================
	// VFX 시스템
	// ============================================================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ordo|VFX")
	TObjectPtr<UNiagaraSystem> AttackVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ordo|VFX")
	TObjectPtr<UNiagaraSystem> HitVFXAsset;

	UFUNCTION(BlueprintCallable, Category = "Ordo|VFX")
	void PlayAttackVFX();

	UFUNCTION(BlueprintCallable, Category = "Ordo|VFX")
	void PlayHitVFX();

	// ============================================================
	// 데이터
	// ============================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Data")
	int32 OrdoDataId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Data")
	FString OrdoName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Data")
	EMOrdoType OrdoType = EMOrdoType::Axiom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Data")
	TArray<int32> SkillIDs;

	/** 무기 병과 — 발사하는 탄환 종류 결정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Data")
	EMWeaponClass WeaponClass = EMWeaponClass::HG;

	/** 이 캐릭터가 실제로 스폰하는 총알 BP 클래스.
	 *  BP_Ordo의 Details에서 BP_Bullet을 지정하면 코드에서 자동 등록됩니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ordo|Data")
	TSubclassOf<AMBulletBase> BulletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ordo|Data")
	int32 DropTableID = 0;

	// ---- 전투 ----
	UPROPERTY(BlueprintReadOnly, Category = "Ordo|Combat")
	bool IsAttacking = false;

	/** AI가 호출하는 공격 함수 */
	UFUNCTION(BlueprintCallable, Category = "Ordo|Combat")
	void PerformAttack(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Ordo|Combat")
	void TakeDamageFromPlayer(float Damage, AActor* Attacker = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Ordo|Combat")
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, Category = "Ordo|Drop")
	void SpawnDropItems();

private:
	/** Down 플립북 재생 완료 콜백 → Hidden */
	UFUNCTION()
	void OnDownAnimFinished();
};
