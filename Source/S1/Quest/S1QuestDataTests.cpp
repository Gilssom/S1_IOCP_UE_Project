#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Quest/S1QuestData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuestDataTest,"S1.Quest.Authoring",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuestDataTest::RunTest(const FString&)
{
 auto* Q=NewObject<UDataTable>();Q->RowStruct=FS1QuestDefinitionRow::StaticStruct();
 auto* D=NewObject<UDataTable>();D->RowStruct=FS1DialogueNodeRow::StaticStruct();
 FS1QuestDefinitionRow Quest;Quest.QuestId=1;Quest.Title=FText::FromString(TEXT("비활성 샘플"));Quest.Description=FText::FromString(TEXT("줄바꿈\n따옴표 \"시험\" \\ 경로"));
 Q->AddRow(TEXT("Sample"),Quest);
 FS1DialogueNodeRow Node;Node.NpcId=1;Node.Speaker=FText::FromString(TEXT("안내원"));Node.Body=FText::FromString(TEXT("대사"));Node.NodeId=1;Node.NextNodeId=2;D->AddRow(TEXT("Intro"),Node);
 Node.NodeId=2;Node.Kind=ES1DialogueNodeKind::QuestOffer;Node.NextNodeId=0;Node.QuestId=1;Node.AcceptNodeId=3;Node.DeclineNodeId=4;D->AddRow(TEXT("Offer"),Node);
 Node.NodeId=3;Node.Kind=ES1DialogueNodeKind::End;Node.QuestId=0;Node.AcceptNodeId=Node.DeclineNodeId=0;D->AddRow(TEXT("AcceptedPreview"),Node);
 Node.NodeId=4;D->AddRow(TEXT("Declined"),Node);
 FString Error,Text;TestTrue(TEXT("Valid disabled quest and graph"),FS1QuestData::Validate(Q,D,Error));
 TestTrue(TEXT("Export succeeds"),FS1QuestData::ExportText(Q,D,Text,Error));TestTrue(TEXT("Canonical header"),Text.StartsWith(TEXT("S1QUEST 1\nQUESTS 1\n")));
 TestTrue(TEXT("Quotes escaped"),Text.Contains(TEXT("\\\"시험\\\"")));
 FString Again;FS1QuestData::ExportText(Q,D,Again,Error);TestEqual(TEXT("Deterministic export"),Text,Again);
 FS1DialogueSession Session;
 TestTrue(TEXT("Start"),Session.Start(Q,D,1,1,Error));TestTrue(TEXT("Next reaches offer"),Session.Advance(D));
 TestFalse(TEXT("No real acceptance without explicit preview"),Session.ChoosePreview(D,true,false));
 TestEqual(TEXT("Blocked accept keeps offer"),Session.CurrentNodeId,2);
 TestTrue(TEXT("Preview accepted branch"),Session.ChoosePreview(D,true,true));TestEqual(TEXT("Accept node"),Session.CurrentNodeId,3);
 TestFalse(TEXT("Repeated click cannot advance terminal"),Session.ChoosePreview(D,true,true));
 Session.Start(Q,D,1,1,Error);Session.Advance(D);TestTrue(TEXT("Decline branch"),Session.ChoosePreview(D,false,true));TestEqual(TEXT("Decline node"),Session.CurrentNodeId,4);
 Session.Reset();TestEqual(TEXT("Cleanup clears session"),Session.CurrentNodeId,0);
 TestFalse(TEXT("Wrong NPC start rejected"),Session.Start(Q,D,2,1,Error));
 D->AddRow(TEXT("Duplicate"),Node);TestFalse(TEXT("Duplicate IDs"),FS1QuestData::Validate(Q,D,Error));D->RemoveRow(TEXT("Duplicate"));
 Node.NodeId=4;Node.Kind=ES1DialogueNodeKind::Line;Node.NextNodeId=1;D->AddRow(TEXT("Declined"),Node);
 TestFalse(TEXT("Cycle rejected"),FS1QuestData::Validate(Q,D,Error));Node.NextNodeId=999;D->AddRow(TEXT("Declined"),Node);
 TestFalse(TEXT("Missing node rejected"),FS1QuestData::ExportText(Q,D,Text,Error));TestTrue(TEXT("Invalid export produces no payload"),Text.IsEmpty());
 Node.Kind=ES1DialogueNodeKind::End;Node.NextNodeId=0;Node.NpcId=2;D->AddRow(TEXT("Declined"),Node);TestFalse(TEXT("Cross NPC branch rejected"),FS1QuestData::Validate(Q,D,Error));
 Node.NpcId=1;D->AddRow(TEXT("Declined"),Node);Quest.MonsterDefinitionId=999;Q->AddRow(TEXT("Sample"),Quest);TestFalse(TEXT("Unknown monster"),FS1QuestData::Validate(Q,D,Error));
 Quest.MonsterDefinitionId=1;Quest.RewardItemId=999;Q->AddRow(TEXT("Sample"),Quest);TestFalse(TEXT("Unknown item"),FS1QuestData::Validate(Q,D,Error));
 Quest.RewardItemId=20001;Quest.RewardCount=0;Q->AddRow(TEXT("Sample"),Quest);TestFalse(TEXT("Invalid count"),FS1QuestData::Validate(Q,D,Error));
 return !HasAnyErrors();
}
#endif
