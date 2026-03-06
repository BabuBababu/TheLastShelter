// Copyright TheLastShelter. All Rights Reserved.

#include "Actor/MBulletBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Actor/MEveCharacter.h"
#include "Actor/MOrdoCharacter.h"
#include "ActorComponent/MStatComponent.h"

AMBulletBase::AMBulletBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// --- Collision ---
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(8.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->OnComponentHit.AddDynamic(this, &AMBulletBase::OnHit);
	SetRootComponent(CollisionComp);

	// --- Flipbook (총알 스프라이트) ---
	FlipbookComp = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("FlipbookComp"));
	FlipbookComp->SetupAttachment(CollisionComp);
	FlipbookComp->SetLooping(true);
	FlipbookComp->Stop();

	// --- Trail VFX (나이아가라 — 비행 트레일) ---
	TrailVFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFXComp"));
	TrailVFXComp->SetupAttachment(CollisionComp);
	TrailVFXComp->SetAutoActivate(false);

	// --- Impact VFX (나이아가라 — 피격 이펙트) ---
	ImpactVFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ImpactVFXComp"));
	ImpactVFXComp->SetupAttachment(CollisionComp);
	ImpactVFXComp->SetAutoActivate(false);

	// --- Projectile Movement ---
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = BulletSpeed;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bAutoActivate = false;

	// 기본 비활성
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AMBulletBase::BeginPlay()
{
	Super::BeginPlay();
}

// ============================================================
// Fire / Deactivate
// ============================================================

void AMBulletBase::Fire(const FVector& StartLocation, const FVector& TargetLocation,
                        float InDamage, AActor* InBulletInstigator, EMWeaponClass InWeaponClass)
{
	// 임팩트 대기 중이면 즉시 정리
	if (bWaitingForImpact)
	{
		ImpactVFXComp->Deactivate();
		ImpactVFXComp->OnSystemFinished.RemoveAll(this);
		bWaitingForImpact = false;
	}

	Damage = InDamage;
	BulletInstigator = InBulletInstigator;
	CurrentWeaponClass = InWeaponClass;
	bPoolActive = true;

	// ★ 발사자와의 충돌을 물리 레벨에서 완전 무시
	if (InBulletInstigator)
	{
		CollisionComp->IgnoreActorWhenMoving(InBulletInstigator, true);
	}

	SetActorLocation(StartLocation);

	// 발사 방향 계산
	FVector direction = (TargetLocation - StartLocation).GetSafeNormal();
	if (direction.IsNearlyZero())
	{
		direction = FVector(1.f, 0.f, 0.f);
	}

	// 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// 플립북 표시 & 재생
	FlipbookComp->SetHiddenInGame(false, true);
	if (BulletFlipbook)
	{
		FlipbookComp->SetFlipbook(BulletFlipbook);
		FlipbookComp->SetLooping(true);
		FlipbookComp->PlayFromStart();
	}
	else
	{
		FlipbookComp->Stop();
	}

	// 비행 트레일 나이아가라
	if (TrailVFXAsset)
	{
		TrailVFXComp->SetAsset(TrailVFXAsset);
		TrailVFXComp->Activate(true);
	}
	else
	{
		TrailVFXComp->Deactivate();
	}

	// 임팩트 VFX 비활성 (발사 시점에는 끔)
	ImpactVFXComp->Deactivate();

	// 투사체 이동 시작
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->Activate(true);
	ProjectileMovement->InitialSpeed = BulletSpeed;
	ProjectileMovement->MaxSpeed = BulletSpeed;
	ProjectileMovement->Velocity = direction * BulletSpeed;

	// 스프라이트 방향 회전
	{
		float firingAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(direction.X, direction.Y));
		float rotDeg = firingAngleDeg - SpriteBaseAngle;
		FRotator spriteRot(0.f, 0.f, rotDeg);
		FlipbookComp->SetRelativeRotation(spriteRot);
	}

	// 수명 타이머
	GetWorldTimerManager().SetTimer(
		LifeTimerHandle, this, &AMBulletBase::OnLifeTimeExpired,
		MaxLifeTime, false);

	UE_LOG(LogTemp, Log, TEXT("[Bullet] Fire from %s | 병과=%d dir(%.1f,%.1f) dmg=%.1f"),
		InBulletInstigator ? *InBulletInstigator->GetName() : TEXT("?"),
		(int32)InWeaponClass, direction.X, direction.Y, InDamage);
}

void AMBulletBase::Deactivate()
{
	bPoolActive = false;
	Damage = 0.f;

	// 발사자 무시 목록 초기화
	if (BulletInstigator.IsValid())
	{
		CollisionComp->IgnoreActorWhenMoving(BulletInstigator.Get(), false);
	}
	BulletInstigator.Reset();

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// 플립북 정리
	FlipbookComp->Stop();
	FlipbookComp->SetHiddenInGame(true, true);
	FlipbookComp->SetRelativeRotation(FRotator::ZeroRotator);

	// 나이아가라 정리
	TrailVFXComp->Deactivate();
	ImpactVFXComp->Deactivate();
	ImpactVFXComp->OnSystemFinished.RemoveAll(this);

	// 투사체 정리
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	ProjectileMovement->SetUpdatedComponent(CollisionComp);

	GetWorldTimerManager().ClearTimer(LifeTimerHandle);
	bWaitingForImpact = false;

	SetActorLocation(FVector(0.f, 0.f, -10000.f));
}

// ============================================================
// 충돌 콜백
// ============================================================

void AMBulletBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                         const FHitResult& Hit)
{
	UE_LOG(LogTemp, Log, TEXT("[Bullet] OnHit — PoolActive=%d Other=%s"),
		bPoolActive, OtherActor ? *OtherActor->GetName() : TEXT("null"));

	if (!bPoolActive) return;

	// 발사자 자신에게 맞으면 무시
	if (OtherActor == BulletInstigator.Get()) return;

	// 아군 판별
	if (BulletInstigator.IsValid() && OtherActor)
	{
		bool instigatorIsEve  = BulletInstigator->IsA<AMEveCharacter>();
		bool targetIsEve      = OtherActor->IsA<AMEveCharacter>();
		bool instigatorIsOrdo = BulletInstigator->IsA<AMOrdoCharacter>();
		bool targetIsOrdo     = OtherActor->IsA<AMOrdoCharacter>();

		if ((instigatorIsEve && targetIsEve) || (instigatorIsOrdo && targetIsOrdo))
		{
			return;
		}
	}

	// 대미지 적용
	if (OtherActor)
	{
		if (AMEveCharacter* eve = Cast<AMEveCharacter>(OtherActor))
		{
			eve->TakeDamageFromOrdo(Damage, BulletInstigator.Get());
			UE_LOG(LogTemp, Log, TEXT("[Bullet] Hit Eve '%s' for %.1f damage"),
				*eve->GetName(), Damage);
		}
		else if (AMOrdoCharacter* ordo = Cast<AMOrdoCharacter>(OtherActor))
		{
			ordo->TakeDamageFromPlayer(Damage, BulletInstigator.Get());
			UE_LOG(LogTemp, Log, TEXT("[Bullet] Hit Ordo '%s' for %.1f damage"),
				*ordo->GetName(), Damage);
		}
	}

	// 비행 관련 즉시 정지 (bPoolActive는 유지 — 임팩트 중 풀 재할당 방지)
	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();
	SetActorEnableCollision(false);
	FlipbookComp->Stop();
	FlipbookComp->SetHiddenInGame(true, true);
	TrailVFXComp->Deactivate();
	GetWorldTimerManager().ClearTimer(LifeTimerHandle);

	// 임팩트 VFX 재생
	UE_LOG(LogTemp, Log, TEXT("[Bullet] Pre-Impact — ImpactVFXAsset=%s"),
		ImpactVFXAsset ? *ImpactVFXAsset->GetName() : TEXT("NULL"));

	if (ImpactVFXAsset)
	{
		ImpactVFXComp->SetAsset(ImpactVFXAsset);
		ImpactVFXComp->SetHiddenInGame(false);
		ImpactVFXComp->OnSystemFinished.AddUniqueDynamic(this, &AMBulletBase::OnImpactVFXFinished);
		ImpactVFXComp->Activate(true);
		bWaitingForImpact = true;

		UE_LOG(LogTemp, Log, TEXT("[Bullet] Impact VFX started at (%.0f, %.0f, %.0f)"),
			GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
	}
	else
	{
		// 이펙트 없으면 즉시 풀 회수
		Deactivate();
	}
}

void AMBulletBase::OnImpactVFXFinished(UNiagaraComponent* FinishedComp)
{
	UE_LOG(LogTemp, Log, TEXT("[Bullet] Impact VFX finished (delegate) — deactivating"));
	bWaitingForImpact = false;
	ImpactVFXComp->OnSystemFinished.RemoveAll(this);
	Deactivate();
}

void AMBulletBase::OnLifeTimeExpired()
{
	if (bPoolActive)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Bullet] Lifetime expired — deactivating"));
		Deactivate();
	}
}
