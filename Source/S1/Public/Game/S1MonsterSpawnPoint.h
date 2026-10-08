#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S1MonsterSpawnPoint.generated.h"

/** Public authoring marker shared with the editor exporter. Origin is feet; never spawns gameplay actors. */
UCLASS(Blueprintable)
class S1_API AS1MonsterSpawnPoint : public AActor
{
	GENERATED_BODY()
public:
	AS1MonsterSpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S1|Spawn", meta=(ClampMin="1"))
	int32 SpawnPointId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S1|Spawn")
	bool bEnabled = true;

	// Common authoring field; only registered Dusk definition 1 is currently supported.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="S1|Spawn", meta=(ClampMin="1"))
	int32 MonsterDefinitionId = 1;

#if WITH_EDITOR
	virtual bool IsEditorOnly() const override { return true; }
#endif
};
