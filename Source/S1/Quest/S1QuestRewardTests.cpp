#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Quest/S1QuestSubsystem.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuestRewardTest,"S1.Quest.RewardContract",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuestRewardTest::RunTest(const FString&)
{
 auto* Instance=NewObject<UGameInstance>();auto* Sub=NewObject<US1QuestSubsystem>(Instance);
 Sub->CharacterId=1;Sub->Session=std::string(16,'s');Sub->bSupported=Sub->bRewardSupported=Sub->bReady=true;
 auto& S=Sub->State;S.set_quest_id(1);S.set_status(Protocol::QUEST_READY_TO_TURN_IN);S.set_definition_version(1);S.set_accepted_instance_id(std::string(16,'a'));
 S.set_required(3);S.set_progress(3);S.set_revision(4);S.set_return_npc_id(1);S.set_reward_item_id(20001);S.set_reward_count(3);
 TestTrue(TEXT("Ready at correct NPC may turn in"),Sub->CanTurnIn(1));
 TestFalse(TEXT("Wrong NPC cannot turn in"),Sub->CanTurnIn(2));
 Sub->bRewardSupported=false;TestFalse(TEXT("Legacy server cannot grant"),Sub->CanTurnIn(1));Sub->bRewardSupported=true;
 Sub->PendingTurnIn.set_request_id(std::string(16,'r'));TestFalse(TEXT("Pending cannot double submit"),Sub->CanTurnIn(1));
 Sub->bTurnInUnconfirmed=true;TestTrue(TEXT("Unknown original request may explicitly reconcile"),Sub->CanTurnIn(1));
 auto Paid=S;Paid.set_rewarded(true);Paid.set_revision(5);
 TestTrue(TEXT("Committed paid state accepted"),US1QuestSubsystem::AcceptSnapshot(S,Paid));
 auto Unpaid=Paid;Unpaid.set_rewarded(false);TestFalse(TEXT("Equal revision cannot erase paid state"),US1QuestSubsystem::AcceptSnapshot(Paid,Unpaid));
 TestFalse(TEXT("Earlier revision cannot erase paid state"),US1QuestSubsystem::AcceptSnapshot(Paid,S));
 Sub->State=Paid;TestFalse(TEXT("Paid cannot claim again"),Sub->CanTurnIn(1));
 Sub->ResetSession();TestTrue(TEXT("New session clears pending reward"),Sub->PendingTurnIn.request_id().empty());
 TestFalse(TEXT("Old response session blocked"),Sub->Matches(1,1,std::string(16,'s')));
 return true;
}
#endif
