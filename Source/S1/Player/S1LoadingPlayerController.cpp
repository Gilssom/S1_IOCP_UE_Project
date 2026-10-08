#include "Player/S1LoadingPlayerController.h"

#include "UI/S1LoadingWidget.h"

void AS1LoadingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
		return;

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	LoadingWidget = CreateWidget<US1LoadingWidget>(this, US1LoadingWidget::StaticClass());
	if (LoadingWidget)
	{
		LoadingWidget->AddToViewport(1000);
	}
}
