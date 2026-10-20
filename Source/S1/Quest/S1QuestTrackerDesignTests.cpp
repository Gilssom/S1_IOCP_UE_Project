#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1QuestTrackerWidget.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuestTrackerDesignTest,"S1.Quest.TrackerDesign",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuestTrackerDesignTest::RunTest(const FString&)
{
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("World"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1QuestTrackerWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1QuestTracker.WBP_S1QuestTracker_C"));
 auto* View=Class?CreateWidget<US1QuestTrackerWidget>(World,Class):nullptr;
 if(TestNotNull(TEXT("Authored tracker"),View))
 {
  View->TakeWidget();View->SetQuestText(FText::FromString(TEXT("QuestTitle\nDusk 1 / 3\nSyncStatus")));
  auto* Title=Cast<UTextBlock>(View->GetWidgetFromName(TEXT("TrackerTitleText")));
  auto* Objective=Cast<UTextBlock>(View->GetWidgetFromName(TEXT("TrackerObjectiveText")));
  auto* Status=Cast<UTextBlock>(View->GetWidgetFromName(TEXT("TrackerStatusText")));
  TestTrue(TEXT("Separate title"),Title&&Title->GetText().ToString()==TEXT("QuestTitle"));
  TestTrue(TEXT("Separate objective"),Objective&&Objective->GetText().ToString()==TEXT("Dusk 1 / 3"));
  TestTrue(TEXT("Separate status"),Status&&Status->GetText().ToString()==TEXT("SyncStatus"));
  View->SetQuestText(FText::FromString(TEXT("QuestTitle\nDusk 3 / 3")));
  TestTrue(TEXT("Empty status consumes no layout space"),Status&&Status->GetVisibility()==ESlateVisibility::Collapsed);
  View->SetQuestText(FText::GetEmpty());
  TestTrue(TEXT("Cleared tracker does not retain title"),Title&&Title->GetText().IsEmpty());
 }
 auto* Fallback=CreateWidget<US1QuestTrackerWidget>(World,US1QuestTrackerWidget::StaticClass());
 if(TestNotNull(TEXT("Native fallback"),Fallback))
 {
  Fallback->TakeWidget();Fallback->SetQuestText(FText::FromString(TEXT("LegacyText")));
  const auto* Text=Cast<UTextBlock>(Fallback->GetWidgetFromName(TEXT("QuestText")));
  TestTrue(TEXT("Legacy single-text fallback preserved"),Text&&Text->GetText().ToString()==TEXT("LegacyText"));
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return true;
}
#endif
