// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MDataTypes.h"
#include "MSpawnManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveStarted, int32, WaveIndex, int32, TotalCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCleared, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvasionStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvasionEnded);

class AMOrdoCharacter;
class UMShelterValueManager;
class UDataTable;

/**
 * MSpawnManager
 *
 * 순수 스폰 시스템: 웨이브 기반 오르도 스폰 + 오브젝트 풀링.
 * 침공 여부 결정은 MHardshipManager가 담당하며, 이 매니저는
 * StartInvasionWithWaves()를 통해 외부 명령을 받아 스폰만 수행합니다.
 *
 * 핵심 흐름:
 *   (외부) MHardshipManager → StartInvasionWithWaves → SpawnWave (반복) → EndInvasion
 *
 * 스폰 규모는 MShelterValueManager의 기지 가치에 비례하여 스케일링됩니다.
 * DataTable(FMOrdoSpawnRow)에서 스폰 후보를 가져옵니다.
 */
UCLASS()
class THELASTSHELTER_API UMSpawnManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================
	// 침공 제어
	// ============================================================

	/** 매 Tick마다 PlayerController/GameMode에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void TickSpawn(float DeltaSeconds);

	/** 외부(MHardshipManager)에서 웨이브 정의를 전달받아 침공 시작 */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void StartInvasionWithWaves(const TArray<FMWaveDefinition>& InWaves);

	/** 침공 강제 시작 (디버그용: 기본 WaveDefinitions 사용) */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ForceStartInvasion();

	/** 침공 강제 종료 */
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void ForceEndInvasion();

	/** 현재 침공 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "Spawn")
	bool IsInvasionActive() const { return InvasionActive; }

	// ============================================================
	// 설정
	// ============================================================

	/** 오르도 스폰 데이터 테이블 (에디터에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Config")
	TObjectPtr<UDataTable> OrdoSpawnTable;

	/** 스폰 위치 (월드 좌표 배열, 에디터에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Config")
	TArray<FVector> SpawnLocations;

	/** 기본 웨이브 정의 (ForceStartInvasion 시 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Config")
	TArray<FMWaveDefinition> WaveDefinitions;

	/** 오브젝트 풀 초기 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Pool")
	int32 PoolInitialSize = 10;

	/** 풀 최대 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Pool")
	int32 PoolMaxSize = 50;

	/** 사망 후 디스폰까지 지연 시간(초). Down 애니메이션 감상 + 드롭 아이템 표시용. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Pool")
	float DespawnDelay = 1.5f;

	// ============================================================
	// 조회
	// ============================================================

	UFUNCTION(BlueprintPure, Category = "Spawn")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	UFUNCTION(BlueprintPure, Category = "Spawn")
	int32 GetAliveOrdoCount() const;

	// ============================================================
	// 이벤트
	// ============================================================

	UPROPERTY(BlueprintAssignable, Category = "Spawn|Event")
	FOnInvasionStarted OnInvasionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spawn|Event")
	FOnInvasionEnded OnInvasionEnded;

	UPROPERTY(BlueprintAssignable, Category = "Spawn|Event")
	FOnWaveStarted OnWaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Spawn|Event")
	FOnWaveCleared OnWaveCleared;

private:
	// ============================================================
	// 침공 로직
	// ============================================================

	void StartInvasion();
	void EndInvasion();
	void SpawnWave();

	// ============================================================
	// 오브젝트 풀
	// ============================================================

	/** 풀에서 비활성 오르도 가져오기 (없으면 새 스폰) */
	AMOrdoCharacter* AcquireFromPool(TSubclassOf<AMOrdoCharacter> OrdoClass);

	/** 오르도를 풀에 반납 (비활성화) */
	void ReturnToPool(AMOrdoCharacter* Ordo);

	/** 풀 초기화 */
	void InitializePool();

	/** 오르도 사망 콜백 */
	UFUNCTION()
	void OnOrdoDeath();

	// ============================================================
	// 내부 상태
	// ============================================================

	bool InvasionActive = false;
	int32 CurrentWaveIndex = -1;
	float WaveTimer = 0.f;
	bool WaitingForNextWave = false;

	/** 현재 침공에서 사용 중인 웨이브 정의 */
	TArray<FMWaveDefinition> ActiveWaveDefinitions;

	/** 현재 웨이브에서 살아있는 오르도 목록 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AMOrdoCharacter>> ActiveOrdos;

	/** 오브젝트 풀 (비활성화 상태) */
	UPROPERTY()
	TArray<TObjectPtr<AMOrdoCharacter>> PooledOrdos;

	// ============================================================
	// 스폰 헬퍼
	// ============================================================

	/** 현재 기지 가치에 적합한 스폰 행을 가중치 기반으로 선택 */
	const FMOrdoSpawnRow* SelectSpawnRow(float BaseValue) const;

	/** 기지 가치 기반 스폰 수 계산 */
	int32 CalculateSpawnCount(const FMWaveDefinition& WaveDef, float BaseValue) const;

	/** 랜덤 스폰 위치 선택 */
	FVector GetRandomSpawnLocation() const;
};
