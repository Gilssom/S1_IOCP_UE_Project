#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Quest/S1QuestSubsystem.h"
#include "Engine/GameInstance.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuestContractTest,"S1.Quest.ServerContract",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuestContractTest::RunTest(const FString&)
{
 Protocol::QuestStateInfo Empty,First;First.set_quest_id(1);
 TestTrue(TEXT("Ready query may confirm not accepted"),US1QuestSubsystem::AcceptSnapshot(Empty,First));
 First.set_status(Protocol::QUEST_ACTIVE);First.set_definition_version(1);First.set_accepted_instance_id(std::string(16,'a'));First.set_required(3);First.set_revision(1);
 TestTrue(TEXT("Committed accepted snapshot"),US1QuestSubsystem::AcceptSnapshot(Empty,First));
 auto New=First;New.set_revision(3);New.set_progress(2);
 TestFalse(TEXT("Older reply cannot replace latest progress"),US1QuestSubsystem::AcceptSnapshot(New,First));
 TestTrue(TEXT("New revision allowed"),US1QuestSubsystem::AcceptSnapshot(First,New));
 New.set_status(Protocol::QUEST_READY_TO_TURN_IN);TestFalse(TEXT("Ready requires goal"),US1QuestSubsystem::AcceptSnapshot(First,New));
 New.set_progress(3);TestTrue(TEXT("Goal reached snapshot"),US1QuestSubsystem::AcceptSnapshot(First,New));
 New.set_progress(4);TestFalse(TEXT("Progress overflow invalid"),US1QuestSubsystem::AcceptSnapshot(First,New));
 New=First;New.set_accepted_instance_id("");TestFalse(TEXT("Active requires accepted identity"),US1QuestSubsystem::AcceptSnapshot(Empty,New));
 auto* Instance=NewObject<UGameInstance>();auto* Sub=NewObject<US1QuestSubsystem>(Instance);
 Sub->CharacterId=10;Sub->Session=std::string(16,'s');Sub->bSupported=true;
 TestTrue(TEXT("Current character/session matched"),Sub->Matches(10,1,std::string(16,'s')));
 TestFalse(TEXT("Prior session rejected"),Sub->Matches(10,1,std::string(16,'p')));
 TestFalse(TEXT("Different character rejected"),Sub->Matches(20,1,std::string(16,'s')));
 Sub->State=First;Sub->bReady=true;Sub->ResetSession();
 TestFalse(TEXT("Session reset drops authority cache"),Sub->bReady);
 TestFalse(TEXT("Session reset prevents late response"),Sub->Matches(10,1,std::string(16,'s')));
 return true;
}
#endif
