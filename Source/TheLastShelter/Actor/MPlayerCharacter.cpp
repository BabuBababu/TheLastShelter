// Copyright TheLastShelter. All Rights Reserved.

#include "MPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMPlayerCharacter::AMPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 2.5D 뷰를 위한 스프링암 + 카메라 설정
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 800.f;
	SpringArmComp->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 5.f;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);

	// 캐릭터 무브먼트 기본값
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	bUseControllerRotationYaw = false;
}

void AMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// EnhancedInput 매핑 컨텍스트 등록
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AMPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이동 입력 → 캐릭터 무브먼트
	if (!MoveInput.IsNearlyZero())
	{
		const FVector WorldDir(MoveInput.X, MoveInput.Y, 0.f);
		AddMovementInput(WorldDir, 1.f);
	}

	// 줌 보간 (BaseArmLength + ZoomIn 오프셋 합산)
	if (SpringArmComp)
	{
		const float currentLength = SpringArmComp->TargetArmLength;
		const float offset = IsZoomedIn ? ZoomInOffset : 0.f;
		const float target = FMath::Clamp(BaseArmLength - offset, MinArmLength, MaxArmLength);
		if (!FMath::IsNearlyEqual(currentLength, target, 0.5f))
		{
			SpringArmComp->TargetArmLength = FMath::FInterpTo(currentLength, target, DeltaTime, ZoomInterpSpeed);
		}
	}
}

void AMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC) return;

	// 이동 (WASD)
	if (IA_Move)
	{
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AMPlayerCharacter::OnMove);
		EIC->BindAction(IA_Move, ETriggerEvent::Completed, this, &AMPlayerCharacter::OnMoveCompleted);
	}

	// 공격 (마우스 왼쪽)
	if (IA_Attack)
		EIC->BindAction(IA_Attack, ETriggerEvent::Started, this, &AMPlayerCharacter::OnAttack);

	// 상호작용 (E)
	if (IA_Interact)
		EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &AMPlayerCharacter::OnInteract);

	// 장전 (R)
	if (IA_Reload)
		EIC->BindAction(IA_Reload, ETriggerEvent::Started, this, &AMPlayerCharacter::OnReload);

	// 메뉴/ESC
	if (IA_Menu)
		EIC->BindAction(IA_Menu, ETriggerEvent::Started, this, &AMPlayerCharacter::OnMenu);

	// 인벤토리 (I)
	if (IA_Inventory)
		EIC->BindAction(IA_Inventory, ETriggerEvent::Started, this, &AMPlayerCharacter::OnInventory);

	// 상태창 (C)
	if (IA_Status)
		EIC->BindAction(IA_Status, ETriggerEvent::Started, this, &AMPlayerCharacter::OnStatus);

	// 줌인 (마우스 오른쪽 홀드)
	if (IA_ZoomIn)
	{
		EIC->BindAction(IA_ZoomIn, ETriggerEvent::Started, this, &AMPlayerCharacter::OnZoomIn);
		EIC->BindAction(IA_ZoomIn, ETriggerEvent::Completed, this, &AMPlayerCharacter::OnZoomIn);
	}

	// 마우스 휠 줌 (휠은 1프레임 펄스이므로 Started 사용)
	if (IA_Zoom)
		EIC->BindAction(IA_Zoom, ETriggerEvent::Started, this, &AMPlayerCharacter::OnZoom);
}

// ============================================================
// Input Handlers
// ============================================================

void AMPlayerCharacter::OnMove(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector2D>();
}

void AMPlayerCharacter::OnMoveCompleted(const FInputActionValue& Value)
{
	MoveInput = FVector2D::ZeroVector;
}

void AMPlayerCharacter::OnAttack(const FInputActionValue& Value)
{
	if (IsAttacking) return;
	IsAttacking = true;

	// TODO: PaperZD AnimInstance에서 공격 애니메이션 재생
	// GetAnimInstance()->PlayAnimationOverride(...)

	UE_LOG(LogTemp, Log, TEXT("[Player] Attack!"));
}

void AMPlayerCharacter::OnInteract(const FInputActionValue& Value)
{
	AActor* Target = GetNearestInteractable();
	if (!Target) return;

	UE_LOG(LogTemp, Log, TEXT("[Player] Interact with: %s"), *Target->GetName());

	// TODO: 루팅/말걸기 분기 처리
}

void AMPlayerCharacter::OnReload(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("[Player] Reload"));
	// TODO: 장전 로직
}

void AMPlayerCharacter::OnMenu(const FInputActionValue& Value)
{
	// 인벤토리/상태창이 열려있으면 닫기, 아니면 메뉴 열기
	if (InventoryOpen)
	{
		InventoryOpen = false;
		UE_LOG(LogTemp, Log, TEXT("[Player] Close Inventory"));
		return;
	}
	if (StatusOpen)
	{
		StatusOpen = false;
		UE_LOG(LogTemp, Log, TEXT("[Player] Close Status"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Player] Open Menu"));
	// TODO: 종합 메뉴 위젯 토글
}

void AMPlayerCharacter::OnInventory(const FInputActionValue& Value)
{
	InventoryOpen = !InventoryOpen;
	UE_LOG(LogTemp, Log, TEXT("[Player] Inventory %s"), InventoryOpen ? TEXT("Open") : TEXT("Close"));
	// TODO: 인벤토리 위젯 토글
}

void AMPlayerCharacter::OnStatus(const FInputActionValue& Value)
{
	StatusOpen = !StatusOpen;
	UE_LOG(LogTemp, Log, TEXT("[Player] Status %s"), StatusOpen ? TEXT("Open") : TEXT("Close"));
	// TODO: 상태창 위젯 토글
}

void AMPlayerCharacter::OnZoomIn(const FInputActionValue& Value)
{
	IsZoomedIn = Value.Get<bool>();
	UE_LOG(LogTemp, Log, TEXT("[Player] ZoomIn: %s"), IsZoomedIn ? TEXT("On") : TEXT("Off"));
}

void AMPlayerCharacter::OnZoom(const FInputActionValue& Value)
{
	const float axisValue = Value.Get<float>();
	BaseArmLength = FMath::Clamp(BaseArmLength - axisValue * ZoomStep, MinArmLength, MaxArmLength);
	UE_LOG(LogTemp, Verbose, TEXT("[Player] Zoom BaseArm=%.0f"), BaseArmLength);
}

// ============================================================
// Interaction
// ============================================================

AActor* AMPlayerCharacter::GetNearestInteractable() const
{
	TArray<FHitResult> Hits;
	const FVector Start = GetActorLocation();
	const FCollisionShape Shape = FCollisionShape::MakeSphere(InteractionRange);

	if (GetWorld()->SweepMultiByChannel(Hits, Start, Start, FQuat::Identity,
		ECollisionChannel::ECC_WorldDynamic, Shape))
	{
		AActor* Nearest = nullptr;
		float MinDist = InteractionRange;

		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor || HitActor == this) continue;

			// MItemActor 또는 MLootingActor 태그 확인
			if (HitActor->Tags.Contains(TEXT("Interactable")))
			{
				const float Dist = FVector::Dist(Start, HitActor->GetActorLocation());
				if (Dist < MinDist)
				{
					MinDist = Dist;
					Nearest = HitActor;
				}
			}
		}
		return Nearest;
	}
	return nullptr;
}
