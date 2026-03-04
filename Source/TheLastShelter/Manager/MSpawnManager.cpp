// Copyright TheLastShelter. All Rights Reserved.

#include "MSpawnManager.h"
#include "MShelterValueManager.h"
#include "MDataManager.h"
#include "MOrdoCharacter.h"
#include "MStatComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

void UMSpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<UMShelterValueManager>();
	Collection.InitializeDependency<UMDataManager>();

	// 기본 웨이브 정의 (비어 있을 때)
	if (WaveDefinitions.Num() == 0)
	{
		FMWaveDefinition defaultWave;
		defaultWave.MinBaseValue = 0.f;
		defaultWave.MinSpawnCount = 2;
		defaultWave.MaxSpawnCount = 5;
		defaultWave.ExtraSpawnPerBaseValue = 0.01f;
		defaultWave.WaveInterval = 10.f;
		WaveDefinitions.Add(defaultWave);
	}

	UE_LOG(LogTemp, Log, TEXT("[SpawnManager] Initialized (%d default wave definitions)"), WaveDefinitions.Num());
}

void UMSpawnManager::Deinitialize()
{
	// 풀 정리
	for (AMOrdoCharacter* ordo : PooledOrdos)
	{
		if (ordo && ordo->IsValidLowLevel())
		{
			ordo->Destroy();
		}
	}
	PooledOrdos.Empty();
	ActiveOrdos.Empty();

	Super::Deinitialize();
}

// ============================================================
// Tick
// ============================================================

void UMSpawnManager::TickSpawn(float DeltaSeconds)
{
	if (!InvasionActive) return;

	// 웨이브 간 대기 타이머
	if (WaitingForNextWave)
	{
		WaveTimer -= DeltaSeconds;
		if (WaveTimer <= 0.f)
		{
			WaitingForNextWave = false;
			SpawnWave();
		}
		return;
	}

	// 현재 웨이브의 생존 오르도 수 검사
	if (GetAliveOrdoCount() <= 0 && CurrentWaveIndex >= 0)
	{
		OnWaveCleared.Broadcast(CurrentWaveIndex);
		UE_LOG(LogTemp, Log, TEXT("[SpawnManager] Wave %d cleared"), CurrentWaveIndex);

		// 다음 웨이브가 있으면 대기, 아니면 침공 종료
		if (CurrentWaveIndex + 1 < ActiveWaveDefinitions.Num())
		{
			WaitingForNextWave = true;
			WaveTimer = ActiveWaveDefinitions[CurrentWaveIndex].WaveInterval;
		}
		else
		{
			EndInvasion();
		}
	}
}

// ============================================================
// 침공 로직
// ============================================================

void UMSpawnManager::StartInvasionWithWaves(const TArray<FMWaveDefinition>& InWaves)
{
	if (InvasionActive) return;

	ActiveWaveDefinitions = InWaves;
	StartInvasion();
}

void UMSpawnManager::ForceStartInvasion()
{
	if (!InvasionActive)
	{
		ActiveWaveDefinitions = WaveDefinitions;
		StartInvasion();
	}
}

void UMSpawnManager::ForceEndInvasion()
{
	if (InvasionActive)
	{
		// 모든 활성 오르도 제거
		for (TWeakObjectPtr<AMOrdoCharacter>& weakOrdo : ActiveOrdos)
		{
			if (AMOrdoCharacter* ordo = weakOrdo.Get())
			{
				ReturnToPool(ordo);
			}
		}
		ActiveOrdos.Empty();
		EndInvasion();
	}
}

void UMSpawnManager::StartInvasion()
{
	InvasionActive = true;
	CurrentWaveIndex = -1;
	WaitingForNextWave = false;

	// ActiveWaveDefinitions 가 비어 있으면 기본값 사용
	if (ActiveWaveDefinitions.Num() == 0)
	{
		ActiveWaveDefinitions = WaveDefinitions;
	}

	// 기지 가치 갱신
	UMShelterValueManager* shelterMgr = GetGameInstance()->GetSubsystem<UMShelterValueManager>();
	if (shelterMgr) shelterMgr->RecalculateBaseValue();

	OnInvasionStarted.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[SpawnManager] === INVASION STARTED (%d waves) ==="), ActiveWaveDefinitions.Num());

	// 첫 웨이브 즉시 시작
	SpawnWave();
}

void UMSpawnManager::EndInvasion()
{
	InvasionActive = false;
	CurrentWaveIndex = -1;
	WaitingForNextWave = false;
	ActiveWaveDefinitions.Empty();

	OnInvasionEnded.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("[SpawnManager] === INVASION ENDED ==="));
}

void UMSpawnManager::SpawnWave()
{
	CurrentWaveIndex++;

	if (CurrentWaveIndex >= ActiveWaveDefinitions.Num())
	{
		EndInvasion();
		return;
	}

	const FMWaveDefinition& waveDef = ActiveWaveDefinitions[CurrentWaveIndex];

	UMShelterValueManager* shelterMgr = GetGameInstance()->GetSubsystem<UMShelterValueManager>();
	float baseValue = shelterMgr ? shelterMgr->GetBaseValue() : 0.f;

	// 기지 가치가 웨이브 최소 조건 미달 시 건너뛰기
	if (baseValue < waveDef.MinBaseValue)
	{
		UE_LOG(LogTemp, Log, TEXT("[SpawnManager] Wave %d skipped (baseValue %.0f < min %.0f)"),
			CurrentWaveIndex, baseValue, waveDef.MinBaseValue);
		if (CurrentWaveIndex + 1 < ActiveWaveDefinitions.Num())
		{
			SpawnWave();
		}
		else
		{
			EndInvasion();
		}
		return;
	}

	int32 spawnCount = CalculateSpawnCount(waveDef, baseValue);

	UE_LOG(LogTemp, Log, TEXT("[SpawnManager] Wave %d: spawning %d Ordos (baseValue=%.0f)"),
		CurrentWaveIndex, spawnCount, baseValue);

	OnWaveStarted.Broadcast(CurrentWaveIndex, spawnCount);

	for (int32 i = 0; i < spawnCount; ++i)
	{
		const FMOrdoSpawnRow* spawnRow = SelectSpawnRow(baseValue);
		if (!spawnRow) continue;

		// 클래스를 로드
		UClass* ordoClass = spawnRow->OrdoClass.LoadSynchronous();
		if (!ordoClass)
		{
			ordoClass = AMOrdoCharacter::StaticClass();
		}

		AMOrdoCharacter* ordo = AcquireFromPool(ordoClass);
		if (!ordo) continue;

		// 위치 설정
		FVector spawnLoc = GetRandomSpawnLocation();
		ordo->SetActorLocation(spawnLoc);

		// 데이터 초기화
		if (spawnRow->OrdoDataId > 0)
		{
			ordo->InitializeFromData(spawnRow->OrdoDataId);
		}

		// StatComponent 연동
		UMStatComponent* statComp = ordo->FindComponentByClass<UMStatComponent>();
		if (statComp)
		{
			statComp->OnDeath.AddDynamic(this, &UMSpawnManager::OnOrdoDeath);
		}

		ActiveOrdos.Add(ordo);
	}
}

// ============================================================
// 오르도 사망 콜백
// ============================================================

void UMSpawnManager::OnOrdoDeath()
{
	// 죽은 오르도를 활성 목록에서 제거하고 풀 반납
	for (int32 i = ActiveOrdos.Num() - 1; i >= 0; --i)
	{
		AMOrdoCharacter* ordo = ActiveOrdos[i].Get();
		if (!ordo || ordo->IsDead())
		{
			if (ordo)
			{
				ReturnToPool(ordo);
			}
			ActiveOrdos.RemoveAt(i);
		}
	}
}

// ============================================================
// 오브젝트 풀
// ============================================================

void UMSpawnManager::InitializePool()
{
	UWorld* world = GetGameInstance()->GetWorld();
	if (!world) return;

	for (int32 i = 0; i < PoolInitialSize; ++i)
	{
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AMOrdoCharacter* ordo = world->SpawnActor<AMOrdoCharacter>(
			AMOrdoCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, params);

		if (ordo)
		{
			ordo->SetActorHiddenInGame(true);
			ordo->SetActorEnableCollision(false);
			ordo->SetActorTickEnabled(false);
			PooledOrdos.Add(ordo);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[SpawnManager] Pool initialized: %d actors"), PooledOrdos.Num());
}

AMOrdoCharacter* UMSpawnManager::AcquireFromPool(TSubclassOf<AMOrdoCharacter> OrdoClass)
{
	// 풀에서 같은 클래스의 비활성 오르도 검색
	for (int32 i = 0; i < PooledOrdos.Num(); ++i)
	{
		AMOrdoCharacter* pooled = PooledOrdos[i];
		if (pooled && pooled->GetClass() == OrdoClass)
		{
			PooledOrdos.RemoveAt(i);

			pooled->SetActorHiddenInGame(false);
			pooled->SetActorEnableCollision(true);
			pooled->SetActorTickEnabled(true);

			return pooled;
		}
	}

	// 풀에 없으면 새로 생성
	UWorld* world = GetGameInstance()->GetWorld();
	if (!world) return nullptr;

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMOrdoCharacter* ordo = world->SpawnActor<AMOrdoCharacter>(
		OrdoClass, FVector::ZeroVector, FRotator::ZeroRotator, params);

	return ordo;
}

void UMSpawnManager::ReturnToPool(AMOrdoCharacter* Ordo)
{
	if (!Ordo) return;

	Ordo->SetActorHiddenInGame(true);
	Ordo->SetActorEnableCollision(false);
	Ordo->SetActorTickEnabled(false);

	// 풀 최대 크기 이내면 보관, 아니면 파괴
	if (PooledOrdos.Num() < PoolMaxSize)
	{
		PooledOrdos.Add(Ordo);
	}
	else
	{
		Ordo->Destroy();
	}
}

int32 UMSpawnManager::GetAliveOrdoCount() const
{
	int32 count = 0;
	for (const TWeakObjectPtr<AMOrdoCharacter>& weakOrdo : ActiveOrdos)
	{
		if (AMOrdoCharacter* ordo = weakOrdo.Get())
		{
			if (!ordo->IsDead())
			{
				count++;
			}
		}
	}
	return count;
}

// ============================================================
// 스폰 헬퍼
// ============================================================

const FMOrdoSpawnRow* UMSpawnManager::SelectSpawnRow(float BaseValue) const
{
	if (!OrdoSpawnTable) return nullptr;

	// 기지 가치에 맞는 행 필터링
	TArray<const FMOrdoSpawnRow*> candidates;
	TArray<float> weights;

	const TMap<FName, uint8*>& rowMap = OrdoSpawnTable->GetRowMap();
	for (const auto& pair : rowMap)
	{
		const FMOrdoSpawnRow* row = reinterpret_cast<const FMOrdoSpawnRow*>(pair.Value);
		if (!row) continue;

		// 기지 가치 범위 확인
		if (BaseValue < row->MinBaseValue) continue;
		if (row->MaxBaseValue > 0.f && BaseValue > row->MaxBaseValue) continue;

		candidates.Add(row);
		weights.Add(row->SpawnWeight);
	}

	if (candidates.Num() == 0) return nullptr;

	// 가중치 랜덤 선택
	float totalWeight = 0.f;
	for (float w : weights) totalWeight += w;

	float roll = FMath::FRand() * totalWeight;
	float accumulated = 0.f;

	for (int32 i = 0; i < candidates.Num(); ++i)
	{
		accumulated += weights[i];
		if (roll <= accumulated)
		{
			return candidates[i];
		}
	}

	return candidates.Last();
}

int32 UMSpawnManager::CalculateSpawnCount(const FMWaveDefinition& WaveDef, float BaseValue) const
{
	int32 baseCount = FMath::RandRange(WaveDef.MinSpawnCount, WaveDef.MaxSpawnCount);
	int32 extraCount = FMath::FloorToInt(BaseValue * WaveDef.ExtraSpawnPerBaseValue);
	return FMath::Max(1, baseCount + extraCount);
}

FVector UMSpawnManager::GetRandomSpawnLocation() const
{
	if (SpawnLocations.Num() == 0)
	{
		// 스폰 위치가 없으면 원점 근처 랜덤
		return FVector(FMath::RandRange(-500.f, 500.f), FMath::RandRange(-500.f, 500.f), 0.f);
	}

	int32 index = FMath::RandRange(0, SpawnLocations.Num() - 1);
	FVector baseLoc = SpawnLocations[index];

	// 약간의 랜덤 오프셋
	baseLoc.X += FMath::RandRange(-100.f, 100.f);
	baseLoc.Y += FMath::RandRange(-100.f, 100.f);

	return baseLoc;
}
