#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1LoadingWidget.generated.h"

UCLASS()
class S1_API US1LoadingWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
};
