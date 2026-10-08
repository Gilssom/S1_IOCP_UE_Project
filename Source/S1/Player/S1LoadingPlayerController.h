#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "S1LoadingPlayerController.generated.h"

UCLASS()
class S1_API AS1LoadingPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<class US1LoadingWidget> LoadingWidget;
};
