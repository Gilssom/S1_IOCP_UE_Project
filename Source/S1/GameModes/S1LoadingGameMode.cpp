#include "GameModes/S1LoadingGameMode.h"

#include "Player/S1LoadingPlayerController.h"
#include "S1GameInstance.h"
#include "TimerManager.h"

AS1LoadingGameMode::AS1LoadingGameMode()
{
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;
	PlayerControllerClass = AS1LoadingPlayerController::StaticClass();
}

void AS1LoadingGameMode::BeginPlay()
{
	Super::BeginPlay();
	if(US1GameInstance* Instance=GetGameInstance<US1GameInstance>())Instance->PortalLoadingStarted();

	GetWorldTimerManager().SetTimerForNextTick([this]()
	{
		FTimerHandle TravelTimer;
		GetWorldTimerManager().SetTimer(
			TravelTimer,
			this,
			&AS1LoadingGameMode::OpenPendingLevel,
			MinimumDisplayTime,
			false);
	});
}

void AS1LoadingGameMode::OpenPendingLevel()
{
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->OpenMainLevel();
	}
}
