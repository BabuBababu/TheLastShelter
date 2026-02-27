// Copyright TheLastShelter. All Rights Reserved.

#include "MAxiomCharacter.h"

AMAxiomCharacter::AMAxiomCharacter()
{
	OrdoType = EMOrdoType::Axiom;
}

void AMAxiomCharacter::BeginPlay()
{
	Super::BeginPlay();

	// TODO: 엑시움 전용 초기화 (기계적 행동 패턴 등)
	UE_LOG(LogTemp, Log, TEXT("[Axiom] %s spawned"), *OrdoName);
}
