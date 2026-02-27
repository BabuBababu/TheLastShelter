// Copyright TheLastShelter. All Rights Reserved.

#include "MSyntheaCharacter.h"

AMSyntheaCharacter::AMSyntheaCharacter()
{
	OrdoType = EMOrdoType::Synthea;
}

void AMSyntheaCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TODO: 신테아 전용 초기화 (감정 통제 패턴 등)
	UE_LOG(LogTemp, Log, TEXT("[Synthea] %s spawned"), *OrdoName);
}
