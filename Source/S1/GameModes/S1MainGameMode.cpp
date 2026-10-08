#include "GameModes/S1MainGameMode.h"

#include "GameFramework/Pawn.h"
#include "S1GameInstance.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AS1MainGameMode::AS1MainGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(
		TEXT("/Game/Blueprints/BP_MyPlayer"));
	if (PlayerPawnClass.Succeeded())
	{
		DefaultPawnClass = PlayerPawnClass.Class;
	}
}

void AS1MainGameMode::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimerForNextTick(
		this,
		&AS1MainGameMode::InitializeMainLevel);
}

void AS1MainGameMode::InitializeMainLevel()
{
	if (US1GameInstance* GameInstance = GetGameInstance<US1GameInstance>())
	{
		GameInstance->CompleteMainLevelEntry();
	}
}
