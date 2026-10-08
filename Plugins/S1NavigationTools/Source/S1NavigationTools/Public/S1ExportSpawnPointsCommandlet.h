#pragma once
#include "Commandlets/Commandlet.h"
#include "S1ExportSpawnPointsCommandlet.generated.h"

UCLASS()
class S1NAVIGATIONTOOLS_API US1ExportSpawnPointsCommandlet final : public UCommandlet
{
	GENERATED_BODY()
public:
	US1ExportSpawnPointsCommandlet();
	virtual int32 Main(const FString& Params) override;
};
