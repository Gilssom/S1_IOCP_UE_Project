#pragma once

#include "CoreMinimal.h"
#include "S1GameModeBase.h"
#include "S1LobbyGameMode.generated.h"

UCLASS()
class S1_API AS1LobbyGameMode : public AS1GameModeBase
{
	GENERATED_BODY()

public:
	AS1LobbyGameMode();

protected:
	virtual void BeginPlay() override;
};
