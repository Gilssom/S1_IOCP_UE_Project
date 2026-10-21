#pragma once
#include "Commandlets/Commandlet.h"
#include "S1ExportMonsterDefinitionsCommandlet.generated.h"
UCLASS()
class S1NAVIGATIONTOOLS_API US1ExportMonsterDefinitionsCommandlet final : public UCommandlet
{
 GENERATED_BODY()
public:
 US1ExportMonsterDefinitionsCommandlet();
 virtual int32 Main(const FString& Params) override;
};
