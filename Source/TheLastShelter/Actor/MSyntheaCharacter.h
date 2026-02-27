// Copyright TheLastShelter. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MOrdoCharacter.h"
#include "MSyntheaCharacter.generated.h"

/**
 * MSyntheaCharacter
 * 신테아(Synthea) - 유전자 결합 신인류. MOrdoCharacter 파생.
 */
UCLASS()
class THELASTSHELTER_API AMSyntheaCharacter : public AMOrdoCharacter
{
	GENERATED_BODY()

public:
	AMSyntheaCharacter();

protected:
	virtual void BeginPlay() override;
};
