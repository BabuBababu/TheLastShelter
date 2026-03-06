// Copyright TheLastShelter. All Rights Reserved.

#include "Manager/MLogManager.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"

// ============================================================
// Subsystem Lifecycle
// ============================================================

void UMLogManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	StartTime = FPlatformTime::Seconds();

	// 로그 파일 경로: {ProjectDir}/Saved/Logs/testlog.txt
	LogFilePath = FPaths::ProjectSavedDir() / TEXT("Logs") / TEXT("testlog.txt");

	// 디렉토리 보장
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(LogFilePath), true);

	// 파일 열기 (매 세션마다 덮어쓰기)
	LogFile.Reset(IFileManager::Get().CreateFileWriter(*LogFilePath, FILEWRITE_None));

	if (LogFile)
	{
		// BOM + 헤더
		FString header = FString::Printf(TEXT("=== TheLastShelter Combat Log — %s ===\n\n"),
			*FDateTime::Now().ToString());
		WriteLineToFile(header);

		UE_LOG(LogTemp, Log, TEXT("[MLogManager] Initialized — %s"), *LogFilePath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[MLogManager] Failed to create log file: %s"), *LogFilePath);
	}
}

void UMLogManager::Deinitialize()
{
	if (LogFile)
	{
		WriteLineToFile(TEXT("\n=== Session End ===\n"));
		LogFile->Flush();
		LogFile->Close();
		LogFile.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("[MLogManager] Deinitialized"));
	Super::Deinitialize();
}

// ============================================================
// Public API
// ============================================================

void UMLogManager::Log(const FString& Category, const FString& Message)
{
	if (!EnableCombatLog) return;

	const double elapsed = FPlatformTime::Seconds() - StartTime;
	const int32 frame = GFrameCounter;

	FString line = FString::Printf(TEXT("[%8.3f | F%d] [%s] %s"),
		elapsed, frame, *Category, *Message);

	// 파일 기록
	WriteLineToFile(line + TEXT("\n"));

	// Output Log에도 동시 출력
	UE_LOG(LogTemp, Log, TEXT("[CombatLog] %s"), *line);
}

void UMLogManager::Logf(const FString& Category, const TCHAR* Fmt, ...)
{
	if (!EnableCombatLog) return;

	TCHAR buffer[2048];
	va_list args;
	va_start(args, Fmt);
	FCString::GetVarArgs(buffer, UE_ARRAY_COUNT(buffer), Fmt, args);
	va_end(args);

	Log(Category, FString(buffer));
}

void UMLogManager::FlushLog()
{
	if (LogFile)
	{
		LogFile->Flush();
	}
}

// ============================================================
// 헬퍼
// ============================================================

FString UMLogManager::ActorID(const AActor* Actor)
{
	if (!Actor) return TEXT("null");
	return FString::Printf(TEXT("%s(%u)"), *Actor->GetName(), Actor->GetUniqueID());
}

// ============================================================
// Internal
// ============================================================

void UMLogManager::WriteLineToFile(const FString& Line)
{
	if (!LogFile) return;

	FTCHARToUTF8 utf8(*Line);
	LogFile->Serialize((void*)utf8.Get(), utf8.Length());

	// 매 100줄마다 자동 플러시 (성능과 안정성 균형)
	static int32 lineCount = 0;
	if (++lineCount % 100 == 0)
	{
		LogFile->Flush();
	}
}
