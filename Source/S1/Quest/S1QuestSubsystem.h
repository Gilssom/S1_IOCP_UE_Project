#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Network/Protocol.pb.h"
#include "S1QuestSubsystem.generated.h"
class US1QuestTrackerWidget;
class US1QuestJournalWidget;
class APlayerController;
enum class ES1ConnectionState:uint8;
/** Character quest cache/request owner. No socket, world registry or rewards. */
UCLASS()
class S1_API US1QuestSubsystem:public UGameInstanceSubsystem
{
 GENERATED_BODY()
 friend class FS1QuestContractTest;
 friend class FS1QuestRewardTest;
 friend class FS1QuestJournalTest;
 friend class FS1UICookReferenceTest;
public:
 US1QuestSubsystem();
 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 virtual void Deinitialize() override;
 bool CanAccept() const;
 bool RequestAccept(uint32 Quest,uint32 Version,uint32 Npc,TFunction<void()> Committed);
 bool CanTurnIn(uint32 Npc) const;
 bool RequestTurnIn(uint32 Npc,TFunction<void()> Committed);
 bool SupportsRewards() const{return bRewardSupported;}
 void ToggleJournal(bool bFromHUD=false);
 void CloseJournal();
 bool IsJournalOpen() const{return Journal!=nullptr;}
 static FString JournalEntry(const Protocol::QuestStateInfo& Snapshot,bool bCompleted);
 FString GetStatusText() const{return StatusText;}
 const Protocol::QuestStateInfo& GetState() const{return State;}
 static bool AcceptSnapshot(const Protocol::QuestStateInfo& Current,const Protocol::QuestStateInfo& Incoming);
private:
 void RefreshJournal();
 void ResetSession();
 void Enter(const Protocol::S_ENTER_GAME& Packet);
 void Leave(const Protocol::S_LEAVE_GAME& Packet);
 void Connection(ES1ConnectionState NewState,const FString& Reason);
 void HandleState(const Protocol::S_QUEST_STATE& Packet);
 void HandleAccept(const Protocol::S_QUEST_ACCEPT_RESULT& Packet);
 void HandleTurnIn(const Protocol::S_QUEST_TURN_IN_RESULT& Packet);
 bool Matches(uint64 Character,uint32 Version,const std::string& Token) const;
 void Query();
 bool Tick(float Delta);
 FString TrackerText() const;
 static std::string NewRequest();
 uint64 CharacterId=0;
 bool bSupported=false,bReady=false,bUnconfirmed=false,bInitialQuery=false;
 bool bRewardSupported=false,bTurnInUnconfirmed=false;
 std::string Session,QueryId;
 Protocol::C_QUEST_ACCEPT PendingAccept;
 Protocol::C_QUEST_TURN_IN PendingTurnIn;
 Protocol::QuestStateInfo State;
 TFunction<void()> OnCommitted;
 TFunction<void()> OnRewardCommitted;
 double TurnInAt=0;
 double QueryAt=0,AcceptAt=0;
 FString StatusText;
 FTSTicker::FDelegateHandle Ticker;
 UPROPERTY() TSubclassOf<US1QuestTrackerWidget> TrackerClass;
 UPROPERTY(Transient) TObjectPtr<US1QuestTrackerWidget> Tracker;
 UPROPERTY() TSubclassOf<US1QuestJournalWidget> JournalClass;
 UPROPERTY(Transient) TObjectPtr<US1QuestJournalWidget> Journal;
 TWeakObjectPtr<APlayerController> JournalOwner;
};
