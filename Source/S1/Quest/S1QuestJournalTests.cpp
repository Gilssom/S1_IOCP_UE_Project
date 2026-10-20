#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Quest/S1QuestSubsystem.h"
#include "UI/S1QuestJournalWidget.h"
#include "UI/S1DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuestJournalTest,"S1.Quest.JournalPolicy",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuestJournalTest::RunTest(const FString&)
{
 auto* Instance=NewObject<UGameInstance>();auto* Sub=NewObject<US1QuestSubsystem>(Instance);
 auto& S=Sub->State;S.set_quest_id(1);S.set_title("DuskQuest");S.set_status(Protocol::QUEST_ACTIVE);S.set_required(3);S.set_progress(1);
 TestTrue(TEXT("Active appears in active list"),Sub->JournalEntry(S,false).Contains(TEXT("DuskQuest")));
 TestFalse(TEXT("Active not in completed list"),Sub->JournalEntry(S,true).Contains(TEXT("DuskQuest")));
 TestFalse(TEXT("Active tracker remains"),Sub->TrackerText().IsEmpty());
 S.set_status(Protocol::QUEST_READY_TO_TURN_IN);S.set_progress(3);
 TestFalse(TEXT("Ready is not completed until reward"),Sub->TrackerText().IsEmpty());
 TestTrue(TEXT("Ready remains in active list"),Sub->JournalEntry(S,false).Contains(TEXT("DuskQuest")));
 S.set_rewarded(true);
 TestTrue(TEXT("Paid removed from tracker"),Sub->TrackerText().IsEmpty());
 TestFalse(TEXT("Paid removed from active list"),Sub->JournalEntry(S,false).Contains(TEXT("DuskQuest")));
 TestTrue(TEXT("Paid appears in completed list"),Sub->JournalEntry(S,true).Contains(TEXT("DuskQuest")));
 S.Clear();TestFalse(TEXT("Empty state cannot invent quest"),Sub->JournalEntry(S,false).Contains(TEXT("DuskQuest")));
 TestFalse(TEXT("Journal initially closed"),Sub->IsJournalOpen());
 Sub->CloseJournal();TestFalse(TEXT("Repeated close safe"),Sub->IsJournalOpen());
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("Test world"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1QuestJournalWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1QuestJournal.WBP_S1QuestJournal_C"));
 auto* View=Class?CreateWidget<US1QuestJournalWidget>(World,Class):nullptr;
 if(TestNotNull(TEXT("Saved journal WBP loads"),View))
 {
  View->TakeWidget();FS1QuestJournalView Entry;Entry.QuestId=1;Entry.Title=FText::FromString(TEXT("QuestEntry"));
  Entry.Description=FText::FromString(TEXT("SavedDescription"));Entry.Objective=FText::FromString(TEXT("Dusk 1/3"));Entry.RewardItemId=20001;Entry.RewardCount=3;
  View->SetQuest(Entry,true);
  TestTrue(TEXT("Active quest appears as list button"),View->QuestListButton&&View->QuestListButton->GetVisibility()==ESlateVisibility::Visible);
  TestTrue(TEXT("Details await selection"),View->DetailFrame&&View->DetailFrame->GetVisibility()==ESlateVisibility::Collapsed);
  View->SelectClicked();
  TestTrue(TEXT("Click populates title"),View->QuestTitleText&&View->QuestTitleText->GetText().ToString()==TEXT("QuestEntry"));
  TestTrue(TEXT("Saved description used"),View->QuestDetailText&&View->QuestDetailText->GetText().ToString().Contains(TEXT("SavedDescription")));
  TestTrue(TEXT("Selected reward shown"),View->RewardIcon&&View->RewardIcon->GetBrush().GetResourceObject()!=nullptr);
  View->SetQuest(Entry,true);TestTrue(TEXT("Snapshot tick retains selection"),View->bSelected);
  View->CompletedClicked();
  TestTrue(TEXT("Empty completed tab hides active row"),View->QuestListButton&&View->QuestListButton->GetVisibility()==ESlateVisibility::Collapsed);
  TestFalse(TEXT("Tab change clears selection"),View->bSelected);
  View->SelectClicked();TestFalse(TEXT("Empty list cannot select hidden quest"),View->bSelected);
  Entry.bCompleted=true;View->SetQuest(Entry,true);
  TestTrue(TEXT("Completed quest appears in completed tab"),View->QuestListButton&&View->QuestListButton->GetVisibility()==ESlateVisibility::Visible);
  View->SelectClicked();TestTrue(TEXT("Completed quest details selectable"),View->bSelected);
  View->ActiveClicked();TestTrue(TEXT("Paid not listed in active tab"),View->QuestListButton&&View->QuestListButton->GetVisibility()==ESlateVisibility::Collapsed);
  View->SetQuest(Entry,false);TestFalse(TEXT("Unready snapshot clears selection"),View->bSelected);
  View->CompletedClicked();View->SelectClicked();TestFalse(TEXT("Unready cannot select"),View->bSelected);
  TestNotNull(TEXT("Close binding"),View->CloseButton.Get());
  int Closes=0;View->OnClose.BindLambda([&]{++Closes;});
  const FGeometry Geometry;
  auto Key=[](FKey K,bool Repeat){return FKeyEvent(K,FModifierKeysState(),0,Repeat,0,0);};
  TestTrue(TEXT("O consumed"),View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::O,false)).IsEventHandled());
  View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::O,true));TestEqual(TEXT("O hold not repeated"),Closes,1);
  View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::Escape,false));TestEqual(TEXT("Esc closes"),Closes,2);
 }
 auto* DialogueClass=LoadClass<US1DialogueWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1Dialogue.WBP_S1Dialogue_C"));
 auto* Dialogue=DialogueClass?CreateWidget<US1DialogueWidget>(World,DialogueClass):nullptr;
 if(TestNotNull(TEXT("Saved dialogue WBP loads"),Dialogue))
 {
  Dialogue->TakeWidget();Dialogue->SetReward(20001,3);
  auto* Frame=Dialogue->GetWidgetFromName(TEXT("RewardFrame"));
  auto* Text=Cast<UTextBlock>(Dialogue->GetWidgetFromName(TEXT("RewardText")));
  auto* Icon=Cast<UImage>(Dialogue->GetWidgetFromName(TEXT("RewardIcon")));
  TestTrue(TEXT("Reward row visible"),Frame&&Frame->GetVisibility()==ESlateVisibility::HitTestInvisible);
  TestTrue(TEXT("Reward quantity bound"),Text&&Text->GetText().ToString().Contains(TEXT("3")));
  TestTrue(TEXT("Existing potion icon bound"),Icon&&Icon->GetBrush().GetResourceObject()!=nullptr);
  Dialogue->SetReward(0,0);TestTrue(TEXT("Ordinary dialogue hides rewards"),Frame&&Frame->GetVisibility()==ESlateVisibility::Collapsed);
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return true;
}
#endif
