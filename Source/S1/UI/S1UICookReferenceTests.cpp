#include "Loot/S1InventorySubsystem.h"
#include "Quest/S1QuestSubsystem.h"
#include "UI/S1QuickSlotWidget.h"
#include "UI/S1QuestTrackerWidget.h"
#include "UI/S1QuestJournalWidget.h"
#include "UObject/UnrealType.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1UICookReferenceTest,"S1.UI.RequiredCookReferences",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1UICookReferenceTest::RunTest(const FString&)
{
 const auto* Inventory=GetDefault<US1InventorySubsystem>();
 const auto* Quest=GetDefault<US1QuestSubsystem>();
 TestNotNull(TEXT("Main HUD uses a hard reflected class property"),FindFProperty<FClassProperty>(Inventory->GetClass(),TEXT("MainHUDClass")));
 TestNotNull(TEXT("Tracker uses a hard reflected class property"),FindFProperty<FClassProperty>(Quest->GetClass(),TEXT("TrackerClass")));
 TestNotNull(TEXT("Journal uses a hard reflected class property"),FindFProperty<FClassProperty>(Quest->GetClass(),TEXT("JournalClass")));
 if(TestNotNull(TEXT("Main HUD native default"),Inventory->MainHUDClass.Get()))
  TestEqual(TEXT("Intended HUD blueprint"),Inventory->MainHUDClass->GetPathName(),FString(TEXT("/Game/S1/UI/WBP_S1MainHUD.WBP_S1MainHUD_C")));
 if(TestNotNull(TEXT("Tracker native default"),Quest->TrackerClass.Get()))
  TestEqual(TEXT("Intended tracker blueprint"),Quest->TrackerClass->GetPathName(),FString(TEXT("/Game/S1/UI/WBP_S1QuestTracker.WBP_S1QuestTracker_C")));
 if(TestNotNull(TEXT("Journal native default"),Quest->JournalClass.Get()))
  TestEqual(TEXT("Intended journal blueprint"),Quest->JournalClass->GetPathName(),FString(TEXT("/Game/S1/UI/WBP_S1QuestJournal.WBP_S1QuestJournal_C")));
 return true;
}
#endif
