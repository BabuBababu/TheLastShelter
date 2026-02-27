// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MOrdoCharacter.h"
#include "MAxiomCharacter.generated.h"

/**
 * MAxiomCharacter
 * 엑시움(Axiom) - AI 군단. MOrdoCharacter 파생.
 */
UCLASS()
class THELASTSHELTER_API AMAxiomCharacter : public AMOrdoCharacter
{
	GENERATED_BODY()

public:
	AMAxiomCharacter();

protected:
	virtual void BeginPlay() override;
};
