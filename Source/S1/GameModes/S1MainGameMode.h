#pragma once

#include "CoreMinimal.h"
#include "S1GameModeBase.h"
#include "S1MainGameMode.generated.h"

UCLASS()
class S1_API AS1MainGameMode : public AS1GameModeBase
{
	GENERATED_BODY()

public:
	AS1MainGameMode();

protected:
	virtual void BeginPlay() override;

private:
	void InitializeMainLevel();
};
