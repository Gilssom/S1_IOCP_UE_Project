#pragma once

#include "Commandlets/Commandlet.h"
#include "S1ExportPortalDataCommandlet.generated.h"

/** Exports saved portal geometry for offline inspection; does not deploy it. */
UCLASS()
class S1NAVIGATIONTOOLS_API US1ExportPortalDataCommandlet final : public UCommandlet
{
	GENERATED_BODY()
public:
	US1ExportPortalDataCommandlet();
	virtual int32 Main(const FString& Params) override;
};
