#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "S1QuickSlotSaveGame.generated.h"

/** Local preferences only. Never persist inventory, HP or credentials here. */
UCLASS()
class S1_API US1QuickSlotSaveGame : public USaveGame
{
 GENERATED_BODY()
public:
 UPROPERTY(SaveGame) int32 Version=1;
 UPROPERTY(SaveGame) FString IdentityHash;
 UPROPERTY(SaveGame) TArray<int32> Definitions;
};
