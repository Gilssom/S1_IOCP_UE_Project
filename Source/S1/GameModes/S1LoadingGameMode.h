#pragma once

#include "CoreMinimal.h"
#include "S1GameModeBase.h"
#include "S1LoadingGameMode.generated.h"

UCLASS()
class S1_API AS1LoadingGameMode : public AS1GameModeBase
{
	GENERATED_BODY()

public:
	AS1LoadingGameMode();

protected:
	virtual void BeginPlay() override;

private:
	void OpenPendingLevel();

	UPROPERTY(EditDefaultsOnly, Category = "Loading", meta = (ClampMin = "0.1"))
	float MinimumDisplayTime = 1.0f;
};
