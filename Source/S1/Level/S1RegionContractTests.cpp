#include "Level/S1RegionContract.h"
#include "Level/S1LevelFlowSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InitialRegionTest,"S1.Level.InitialRegionContract",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InitialRegionTest::RunTest(const FString&)
{
 TestEqual(TEXT("Village whitelist"),FS1RegionContract::MapFor(2),FName(TEXT("/Game/Maps/VillageMap")));
 TestEqual(TEXT("Combat preserved"),FS1RegionContract::MapFor(1),FName(TEXT("/Game/Maps/MainMap")));
 TestTrue(TEXT("Unknown region never falls back"),FS1RegionContract::MapFor(3).IsNone());
 Protocol::S_ENTER_GAME P;P.set_success(true);P.set_region_id(2);P.set_region_contract_version(1);
 auto* Player=P.mutable_player();Player->set_object_id(1);Player->mutable_character_info()->set_character_id(1);Player->mutable_pos_info()->set_z(192);
 TestTrue(TEXT("Valid initial entry"),FS1RegionContract::Supports(P));
 P.set_region_contract_version(0);TestFalse(TEXT("Legacy server denied"),FS1RegionContract::Supports(P));
 P.set_region_contract_version(1);P.set_region_id(0);TestFalse(TEXT("Missing region denied"),FS1RegionContract::Supports(P));
 P.set_region_id(2);P.clear_player();TestFalse(TEXT("Missing player denied"),FS1RegionContract::Supports(P));
 auto* Instance=NewObject<UGameInstance>();
 auto* Flow=NewObject<US1LevelFlowSubsystem>(Instance);
 TestFalse(TEXT("Invalid region not retained"),Flow->SetInitialRegion(999));
 TestTrue(TEXT("Accept Village"),Flow->SetInitialRegion(2));TestEqual(TEXT("Pending region"),Flow->GetInitialRegion(),2u);
 Flow->EnterLobbyState();TestEqual(TEXT("Lobby reset clears region"),Flow->GetInitialRegion(),0u);
 return true;
}
#endif
