// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MLogManager.generated.h"

/**
 * MLogManager — 전투 디버그 로그를 파일(testlog.txt)에 기록하는 서브시스템.
 *
 * ■ 타임스탬프 + 인스턴스 이름/ID로 전투 흐름 완전 추적
 * ■ Eve 전투, Ordo 스폰/사망/디스폰, 총알 발사/피격, AI 태스크 전환 모두 기록
 * ■ 에디터에서 EnableCombatLog = false로 끄면 파일 기록 중단
 */
UCLASS()
class THELASTSHELTER_API UMLogManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ============================================================
	// API
	// ============================================================

	/** 전투 로그 1줄 기록. Category 예: "Eve", "Ordo", "Bullet", "AI", "Spawn" */
	void Log(const FString& Category, const FString& Message);

	/** Printf 스타일 로그 */
	void Logf(const FString& Category, const TCHAR* Fmt, ...);

	/** 로그 활성화 여부 (런타임 토글 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log")
	bool EnableCombatLog = true;

	/** 로그 파일을 수동 플러시 */
	void FlushLog();

	// ============================================================
	// 헬퍼 — 액터 식별 문자열
	// ============================================================

	/** 액터의 이름 + 유니크 ID를 "Name(ID)" 형태로 반환 */
	static FString ActorID(const AActor* Actor);

private:
	/** 파일 핸들 */
	TUniquePtr<FArchive> LogFile;

	/** 로그 파일 경로 */
	FString LogFilePath;

	/** 부팅 시간 (상대 타임스탬프용) */
	double StartTime = 0.0;

	/** 내부 기록 */
	void WriteLineToFile(const FString& Line);
};
