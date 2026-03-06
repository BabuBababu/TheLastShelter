// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MDataTypes.h"
#include "MBulletBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPaperFlipbook;
class UPaperFlipbookComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * MBulletBase — 오브젝트 풀링 기반 총알 액터.
 *
 * ■ 직선 비행 (UProjectileMovementComponent)
 * ■ 충돌 시 대미지 적용 + 자동 비활성화 (풀 회수)
 * ■ MProjectileManager가 대여/회수를 관리
 * ■ 비행 트레일 / 피격 이펙트는 Niagara로 처리
 */
UCLASS()
class THELASTSHELTER_API AMBulletBase : public AActor
{
	GENERATED_BODY()

public:
	AMBulletBase();

	// ============================================================
	// 컴포넌트
	// ============================================================

	/** 충돌 체크용 스피어 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	TObjectPtr<USphereComponent> CollisionComp;

	/** 투사체 이동 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 시각적 표현 (2D 플립북 애니메이션) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet")
	TObjectPtr<UPaperFlipbookComponent> FlipbookComp;

	/** 비행 트레일 나이아가라 (총알에 부착, 비행 중 루프) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|VFX")
	TObjectPtr<UNiagaraComponent> TrailVFXComp;

	/** 피격 이펙트 나이아가라 (총알에 부착, OnHit 시 1회 → 자동 비활성) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bullet|VFX")
	TObjectPtr<UNiagaraComponent> ImpactVFXComp;

	// ============================================================
	// 발사 설정
	// ============================================================

	/**
	 * 총알 발사.
	 * @param StartLocation  시작 위치 (총구)
	 * @param TargetLocation 목표 위치 (명중률 적용된 최종 좌표)
	 * @param InDamage       이 총알이 전달할 대미지
	 * @param InBulletInstigator 발사한 캐릭터
	 * @param InWeaponClass  발사자의 병과
	 */
	void Fire(const FVector& StartLocation, const FVector& TargetLocation,
	          float InDamage, AActor* InBulletInstigator,
	          EMWeaponClass InWeaponClass = EMWeaponClass::HG);

	/** 풀 회수 — 비활성화 후 원점으로 */
	void Deactivate();

	/** 현재 활성 상태인지 */
	bool IsPoolActive() const { return bPoolActive; }

	// ============================================================
	// 설정 (튜닝)
	// ============================================================

	/**
	 * 비행 중 재생할 플립북.
	 * BP_Bullet_AR라면 AR 플립북 1개만 여기에 지정.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Visuals")
	TObjectPtr<UPaperFlipbook> BulletFlipbook;

	/** 비행 트레일 나이아가라 에셋 (루프). 없으면 트레일 없음. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|VFX")
	TObjectPtr<UNiagaraSystem> TrailVFXAsset;

	/** 피격 이펙트 나이아가라 에셋 (1회 버스트). 없으면 이펙트 없음. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|VFX")
	TObjectPtr<UNiagaraSystem> ImpactVFXAsset;

	/**
	 * 스프라이트 아트가 그려진 기준 각도 (도).
	 * 3시 방향 = 0°, 반시계 양수.
	 * 7시 방향 ≈ 225° (기본값).
	 * Fire() 시 발사 방향과의 차이만큼 자동 회전.
	 *
	 * ▶ 회전 중심(피봇) 조정: BP에서 FlipbookComp의 Relative Location을 오프셋.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Visuals")
	float SpriteBaseAngle = 225.f;

	/** 총알 비행 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Config")
	float BulletSpeed = 3000.f;

	/** 최대 비행 시간 (초) — 이 시간이 지나면 자동 회수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Config")
	float MaxLifeTime = 3.f;

protected:
	virtual void BeginPlay() override;

private:
	/** 충돌 콜백 */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	           UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	           const FHitResult& Hit);

	/** 수명 타이머 만료 → 자동 회수 */
	void OnLifeTimeExpired();

	/** ImpactVFX 재생 완료 델리게이트 콜백 */
	UFUNCTION()
	void OnImpactVFXFinished(UNiagaraComponent* FinishedComp);

	/** 풀 활성 상태 */
	bool bPoolActive = false;

	/** 임팩트 재생 대기 중 여부 */
	bool bWaitingForImpact = false;

	/** 이 총알의 대미지 */
	float Damage = 0.f;

	/** 현재 병과 (Fire() 시 설정) */
	EMWeaponClass CurrentWeaponClass = EMWeaponClass::HG;

	/** 발사자 (아군/적 판별용) */
	UPROPERTY()
	TWeakObjectPtr<AActor> BulletInstigator;

	/** 수명 타이머 핸들 */
	FTimerHandle LifeTimerHandle;
};
