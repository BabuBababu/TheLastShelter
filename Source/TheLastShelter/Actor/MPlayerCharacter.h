// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "InputActionValue.h"
#include "MDataTypes.h"
#include "MPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPaperZDAnimInstance;

/**
 * MPlayerCharacter
 * 플레이어 캐릭터. APaperZDCharacter 상속.
 * EnhancedInput으로 이동/공격/상호작용 처리.
 */
UCLASS()
class THELASTSHELTER_API AMPlayerCharacter : public APaperZDCharacter
{
	GENERATED_BODY() 

public:
	AMPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ---- 카메라 ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;

	// ---- Enhanced Input ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Reload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Menu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Inventory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Status;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ZoomIn;

	/** 마우스 휠 줌 (Axis1D: +1 = 줌인, -1 = 줌아웃) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Zoom;

	// ---- Input Callbacks ----
	void OnMove(const FInputActionValue& Value);
	void OnMoveCompleted(const FInputActionValue& Value);
	void OnAttack(const FInputActionValue& Value);
	void OnInteract(const FInputActionValue& Value);
	void OnReload(const FInputActionValue& Value);
	void OnMenu(const FInputActionValue& Value);
	void OnInventory(const FInputActionValue& Value);
	void OnStatus(const FInputActionValue& Value);
	void OnZoomIn(const FInputActionValue& Value);
	void OnZoom(const FInputActionValue& Value);

public:
	// ---- Stat ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	FMPhysicalStat PhysicalStat;

	// ---- Interaction ----
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetNearestInteractable() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionRange = 200.f;

	// ---- State ----
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool IsAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	FVector2D MoveInput;

	/** 현재 인벤토리 UI 열림 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool InventoryOpen = false;

	/** 현재 상태창 UI 열림 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool StatusOpen = false;

	// ---- Zoom ----

	/** 마우스 줌인 활성 여부 (우클릭 홀드) */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	bool IsZoomedIn = false;

	/** 기본 스프링암 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float DefaultArmLength = 800.f;

	/** 우클릭 줌인 시 추가 줄어드는 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomInOffset = 300.f;

	/** 마우스 휠 줌 최소 스프링암 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinArmLength = 200.f;

	/** 마우스 휠 줌 최대 스프링암 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxArmLength = 1500.f;

	/** 마우스 휠 1클릭당 줌 변화량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomStep = 50.f;

	/** 줌 보간 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomInterpSpeed = 10.f;

private:
	/** 휠 줌으로 설정된 기본 스프링암 길이 */
	float BaseArmLength = 800.f;
};
