#include "GameModes/S1LobbyGameMode.h"

#include "S1GameInstance.h"

AS1LobbyGameMode::AS1LobbyGameMode()
{
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = true;
}

void AS1LobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->EnterLobbyState();
		GameInstance->ShowAuthUI();
		GameInstance->ConnectToGameServer();
	}
}
