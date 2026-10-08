#pragma once

#include "Commandlets/Commandlet.h"
#include "S1ExportNavigationCommandlet.generated.h"

UCLASS()
class S1NAVIGATIONTOOLS_API US1ExportNavigationCommandlet final : public UCommandlet
{
	GENERATED_BODY()

public:
	US1ExportNavigationCommandlet();
	virtual int32 Main(const FString& Params) override;
};
