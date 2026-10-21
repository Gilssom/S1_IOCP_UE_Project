#include "Quest/S1QuestSubsystem.h"
#include "Network/S1NetworkSubsystem.h"
#include "Level/S1LevelFlowSubsystem.h"
#include "UI/S1QuestTrackerWidget.h"
#include "UI/S1QuestJournalWidget.h"
#include "Quest/S1DialogueSubsystem.h"
#include "Game/S1MyPlayer.h"
#include "Loot/S1InventorySubsystem.h"
#include "ClientPacketHandler.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "UObject/ConstructorHelpers.h"

US1QuestSubsystem::US1QuestSubsystem()
{
 // Required packaged UI uses explicit hard references, like inventory widgets.
 static ConstructorHelpers::FClassFinder<US1QuestTrackerWidget> TrackerAsset(TEXT("/Game/S1/UI/WBP_S1QuestTracker"));
 static ConstructorHelpers::FClassFinder<US1QuestJournalWidget> JournalAsset(TEXT("/Game/S1/UI/WBP_S1QuestJournal"));
 TrackerClass=TrackerAsset.Class;JournalClass=JournalAsset.Class;
}
void US1QuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
 Super::Initialize(Collection);Collection.InitializeDependency<US1NetworkSubsystem>();
 auto* N=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 N->OnEnterGamePacket.AddUObject(this,&US1QuestSubsystem::Enter);N->OnLeaveGamePacket.AddUObject(this,&US1QuestSubsystem::Leave);
 N->OnConnectionStateChanged.AddUObject(this,&US1QuestSubsystem::Connection);
 N->OnQuestStatePacket.AddUObject(this,&US1QuestSubsystem::HandleState);N->OnQuestAcceptPacket.AddUObject(this,&US1QuestSubsystem::HandleAccept);
 N->OnQuestTurnInPacket.AddUObject(this,&US1QuestSubsystem::HandleTurnIn);
 Ticker=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&US1QuestSubsystem::Tick),.1f);
}
void US1QuestSubsystem::Deinitialize()
{
 ResetSession();FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
 if(auto* N=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>())
 {N->OnEnterGamePacket.RemoveAll(this);N->OnLeaveGamePacket.RemoveAll(this);N->OnConnectionStateChanged.RemoveAll(this);N->OnQuestStatePacket.RemoveAll(this);N->OnQuestAcceptPacket.RemoveAll(this);N->OnQuestTurnInPacket.RemoveAll(this);}
 Super::Deinitialize();
}
void US1QuestSubsystem::ResetSession()
{
 CloseJournal();
 CharacterId=0;Session.clear();State.Clear();PendingAccept.Clear();QueryId.clear();OnCommitted=nullptr;
 bSupported=bReady=bUnconfirmed=bInitialQuery=false;StatusText.Reset();QueryAt=AcceptAt=0;
 bRewardSupported=bTurnInUnconfirmed=false;PendingTurnIn.Clear();OnRewardCommitted=nullptr;TurnInAt=0;
 if(Tracker){Tracker->RemoveFromParent();Tracker=nullptr;}
}
void US1QuestSubsystem::Enter(const Protocol::S_ENTER_GAME& Packet)
{
 ResetSession();if(!Packet.success())return;
 CharacterId=Packet.player().character_info().character_id();Session=Packet.quest_session_id();
 bSupported=Packet.quest_contract_version()==1&&CharacterId!=0&&Session.size()==16&&Session.find_first_not_of('\0')!=std::string::npos;
 bRewardSupported=bSupported&&Packet.quest_reward_contract_version()==1;
 StatusText=bSupported?TEXT("퀘스트 동기화 대기"):TEXT("서버 퀘스트 기능 미준비");
}
void US1QuestSubsystem::Leave(const Protocol::S_LEAVE_GAME&){ResetSession();}
void US1QuestSubsystem::Connection(ES1ConnectionState NewState,const FString&)
{if(NewState!=ES1ConnectionState::Connected)ResetSession();}
bool US1QuestSubsystem::Matches(uint64 Character,uint32 Version,const std::string& Token) const
{return bSupported&&Character==CharacterId&&Version==1&&Token==Session;}
bool US1QuestSubsystem::AcceptSnapshot(const Protocol::QuestStateInfo& Current,const Protocol::QuestStateInfo& Incoming)
{
 if(Incoming.quest_id()!=1||!Protocol::QuestStatus_IsValid(Incoming.status())||Incoming.revision()<Current.revision())return false;
 if(Incoming.rewarded()&&Incoming.status()!=Protocol::QUEST_READY_TO_TURN_IN)return false;
 if(Current.rewarded()&&Current.accepted_instance_id()==Incoming.accepted_instance_id()&&!Incoming.rewarded())return false;
 if(Incoming.status()==Protocol::QUEST_NOT_ACCEPTED)return Incoming.revision()==0&&Incoming.progress()==0&&Incoming.accepted_instance_id().empty();
 return Incoming.definition_version()>0&&Incoming.accepted_instance_id().size()==16&&Incoming.accepted_instance_id().find_first_not_of('\0')!=std::string::npos&&
  Incoming.revision()>0&&Incoming.required()>0&&Incoming.required()<=100000&&Incoming.progress()<=Incoming.required()&&
  (Incoming.status()==Protocol::QUEST_ACTIVE?Incoming.progress()<Incoming.required():Incoming.progress()==Incoming.required());
}
std::string US1QuestSubsystem::NewRequest()
{
 FGuid Id=FGuid::NewGuid();const uint32 Parts[4]={Id.A,Id.B,Id.C,Id.D};std::string Bytes(16,'\0');
 for(int32 P=0;P<4;++P)for(int32 B=0;B<4;++B)Bytes[P*4+B]=static_cast<char>((Parts[P]>>(8*B))&255);return Bytes;
}
void US1QuestSubsystem::Query()
{
 if(!bSupported)return;auto* N=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();if(!N||!N->IsConnected())return;
 QueryId=NewRequest();Protocol::C_QUEST_QUERY Packet;Packet.set_request_id(QueryId);Packet.set_quest_id(1);Packet.set_contract_version(1);
 QueryAt=FPlatformTime::Seconds();N->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));
}
void US1QuestSubsystem::HandleState(const Protocol::S_QUEST_STATE& Packet)
{
 if(!Matches(Packet.character_id(),Packet.contract_version(),Packet.quest_session_id())||
  (!Packet.request_id().empty()&&Packet.request_id()!=QueryId))return;
 if(!Packet.request_id().empty())QueryId.clear();
 if(Packet.query_status()!=Protocol::QUEST_QUERY_READY){bReady=false;StatusText=TEXT("퀘스트 동기화 확인 중");return;}
 if(!AcceptSnapshot(State,Packet.state()))return;
 const bool bNewlyPaid=!State.rewarded()&&Packet.state().rewarded();
 State=Packet.state();bReady=true;StatusText=TEXT("퀘스트 상태 동기화됨");
 if(bNewlyPaid&&bRewardSupported)
 {if(auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>())Inventory->RefreshAfterQuestReward(0);}
 if(State.rewarded()&&!PendingTurnIn.request_id().empty())
 {PendingTurnIn.Clear();OnRewardCommitted=nullptr;bTurnInUnconfirmed=false;}
 // A query proves accepted state, not which request committed. Never reopen or
 // advance dialogue from this reconciliation; only an exact committed response may.
 if(State.status()!=Protocol::QUEST_NOT_ACCEPTED&&!PendingAccept.request_id().empty())
 {PendingAccept.Clear();OnCommitted=nullptr;bUnconfirmed=false;}
}
bool US1QuestSubsystem::CanAccept() const
{return bSupported&&bReady&&State.status()==Protocol::QUEST_NOT_ACCEPTED&&(PendingAccept.request_id().empty()||bUnconfirmed);}
bool US1QuestSubsystem::RequestAccept(uint32 Quest,uint32 Version,uint32 Npc,TFunction<void()> Committed)
{
 if(!CanAccept()||Quest!=1||!Version||!Npc)return false;
 auto* N=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();if(!N||!N->IsConnected())return false;
 if(PendingAccept.request_id().empty())
 {
  PendingAccept.set_request_id(NewRequest());PendingAccept.set_quest_id(Quest);PendingAccept.set_definition_version(Version);
  PendingAccept.set_npc_id(Npc);PendingAccept.set_contract_version(1);
 }
 else if(PendingAccept.quest_id()!=Quest||PendingAccept.definition_version()!=Version||PendingAccept.npc_id()!=Npc)return false;
 OnCommitted=MoveTemp(Committed);AcceptAt=FPlatformTime::Seconds();bUnconfirmed=false;
 StatusText=TEXT("수락 저장 확인 중");N->SendPacket(ClientPacketHandler::MakeSendBuffer(PendingAccept));return true;
}
void US1QuestSubsystem::HandleAccept(const Protocol::S_QUEST_ACCEPT_RESULT& Packet)
{
 if(!Matches(Packet.character_id(),Packet.contract_version(),Packet.quest_session_id())||!Protocol::QuestAcceptDecision_IsValid(Packet.decision())||!Protocol::QuestAcceptReason_IsValid(Packet.reason()))return;
 if(Packet.has_state()&&AcceptSnapshot(State,Packet.state())){State=Packet.state();bReady=true;}
 if(PendingAccept.request_id().empty()||Packet.request_id()!=PendingAccept.request_id())return;
 UE_LOG(LogTemp,Log,TEXT("Quest accept result. Request[%s] Decision[%d] Reason[%d] Revision[%llu]"),
  *FString::FromHexBlob(reinterpret_cast<const uint8*>(Packet.request_id().data()),static_cast<uint32>(Packet.request_id().size())),static_cast<int32>(Packet.decision()),static_cast<int32>(Packet.reason()),State.revision());
 if(Packet.decision()==Protocol::QUEST_ACCEPT_COMMITTED&&Packet.reason()==Protocol::QUEST_REASON_NONE&&State.status()!=Protocol::QUEST_NOT_ACCEPTED)
 {
  PendingAccept.Clear();bUnconfirmed=false;StatusText=TEXT("퀘스트 수락 완료");
  auto Callback=MoveTemp(OnCommitted);OnCommitted=nullptr;if(Callback)Callback();return;
 }
 if(Packet.decision()==Protocol::QUEST_ACCEPT_REJECTED)
 {PendingAccept.Clear();OnCommitted=nullptr;bUnconfirmed=false;StatusText=FString::Printf(TEXT("수락되지 않음 (사유 %d)"),static_cast<int32>(Packet.reason()));return;}
 if(Packet.decision()==Protocol::QUEST_ACCEPT_UNCONFIRMED)
 {bUnconfirmed=true;StatusText=TEXT("수락 결과 확인 중 · 다시 누르면 동일 요청 재확인");Query();}
}
FString US1QuestSubsystem::TrackerText() const
{
 if(State.rewarded())return FString(); // Remove only after authoritative reward completion.
 if(State.status()==Protocol::QUEST_NOT_ACCEPTED)return bReady?FString():StatusText;
 const TCHAR* Suffix=State.rewarded()?TEXT(" · 보상 수령 완료"):
  State.status()==Protocol::QUEST_READY_TO_TURN_IN?(bRewardSupported?TEXT(" · NPC에게 완료 보고"):TEXT(" · 목표 달성 (보상 기능 미지원)")):TEXT("");
 return FString::Printf(TEXT("%s\nDusk %u / %u%s\n%s"),UTF8_TO_TCHAR(State.title().c_str()),State.progress(),State.required(),Suffix,bReady?TEXT(""):*StatusText);
}
bool US1QuestSubsystem::Tick(float)
{
 if(!bSupported)return true;
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;
 if(!Flow||!Flow->IsMainLevelReady()||!PC||!PC->IsLocalController())
 {CloseJournal();if(Tracker){Tracker->RemoveFromParent();Tracker=nullptr;}return true;}
 const auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>();
 const bool bDialogue=Dialogue&&Dialogue->IsModal();
 if(Journal&&(JournalOwner.Get()!=PC||Journal->GetWorld()!=GetWorld()||bDialogue))CloseJournal();
 RefreshJournal();
 if(!bInitialQuery){bInitialQuery=true;Query();}
 const double Now=FPlatformTime::Seconds();
 if(!PendingAccept.request_id().empty()&&!bUnconfirmed&&Now-AcceptAt>=3.)
 {bUnconfirmed=true;StatusText=TEXT("수락 응답 지연 · 결과 확인 중");Query();}
 if(!PendingTurnIn.request_id().empty()&&!bTurnInUnconfirmed&&Now-TurnInAt>=3.)
 {bTurnInUnconfirmed=true;StatusText=TEXT("보상 응답 지연 · 결과 확인 중");Query();}
 if((!bReady||!QueryId.empty())&&Now-QueryAt>=5.)Query();
 if(!Tracker)
 {auto* Class=TrackerClass.Get();Tracker=CreateWidget<US1QuestTrackerWidget>(PC,Class?Class:US1QuestTrackerWidget::StaticClass());if(Tracker)Tracker->AddToViewport(8);}
 if(Tracker){const auto Text=TrackerText();Tracker->SetQuestText(FText::FromString(Text));Tracker->SetVisibility(bDialogue||Text.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);}
 return true;
}
FString US1QuestSubsystem::JournalEntry(const Protocol::QuestStateInfo& Snapshot,bool bCompleted)
{
 if(Snapshot.status()==Protocol::QUEST_NOT_ACCEPTED||Snapshot.quest_id()==0||Snapshot.rewarded()!=bCompleted)
  return bCompleted?TEXT("완료한 퀘스트가 없습니다."):TEXT("진행 중인 퀘스트가 없습니다.");
 return FString::Printf(TEXT("%s\nDusk %u / %u\n%s"),UTF8_TO_TCHAR(Snapshot.title().c_str()),Snapshot.progress(),Snapshot.required(),
  bCompleted?TEXT("보상 수령 완료"):Snapshot.status()==Protocol::QUEST_READY_TO_TURN_IN?TEXT("목표 달성 · NPC에게 완료 보고"):TEXT("진행 중"));
}
void US1QuestSubsystem::ToggleJournal(bool bFromHUD)
{
 if(Journal){CloseJournal();return;}
 auto* World=GetWorld();auto* PC=World?World->GetFirstPlayerController():nullptr;
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 const auto* Dialogue=World?World->GetSubsystem<US1DialogueSubsystem>():nullptr;
 auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>();
 if(!PC||!PC->IsLocalController()||!Flow||!Flow->IsMainLevelReady()||!bSupported||
  (Dialogue&&Dialogue->IsModal())||(Inventory&&Inventory->IsInventoryOpen())||(!bFromHUD&&PC->bShowMouseCursor))return;
 auto* Class=JournalClass.Get();if(!Class)return;
 Journal=CreateWidget<US1QuestJournalWidget>(PC,Class);if(!Journal)return;
 JournalOwner=PC;Journal->OnClose.BindUObject(this,&US1QuestSubsystem::CloseJournal);
 Journal->AddToViewport(30);
 RefreshJournal();
 if(auto* Player=Cast<AS1MyPlayer>(PC->GetPawn()))Player->StopForDialogue();
 FInputModeUIOnly Input;Input.SetWidgetToFocus(Journal->TakeWidget());PC->SetInputMode(Input);PC->bShowMouseCursor=true;
 Query();
}
void US1QuestSubsystem::CloseJournal()
{
 if(!Journal)return;
 const auto Owner=JournalOwner;Journal->OnClose.Unbind();Journal->RemoveFromParent();Journal=nullptr;JournalOwner.Reset();
 auto* World=GetWorld();const auto* Dialogue=World?World->GetSubsystem<US1DialogueSubsystem>():nullptr;
 const auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>();
 const auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 if(auto* PC=Owner.Get();PC&&World&&!World->bIsTearingDown&&PC->GetWorld()==World&&PC->GetPawn()&&Flow&&Flow->IsMainLevelReady()&&
  !(Dialogue&&Dialogue->IsModal())&&!(Inventory&&Inventory->IsInventoryOpen()))
 {PC->SetInputMode(FInputModeGameOnly());PC->bShowMouseCursor=false;}
}
void US1QuestSubsystem::RefreshJournal()
{
 if(!Journal)return;
 FS1QuestJournalView View;
 if(State.status()!=Protocol::QUEST_NOT_ACCEPTED)
 {
  View.QuestId=State.quest_id();View.bCompleted=State.rewarded();
  View.Title=FText::FromString(UTF8_TO_TCHAR(State.title().c_str()));
  View.Description=FText::FromString(UTF8_TO_TCHAR(State.description().c_str()));
  View.Objective=FText::FromString(FString::Printf(TEXT("목표: Dusk 처치 %u / %u"),State.progress(),State.required()));
  View.StateText=FText::FromString(State.rewarded()?TEXT("완료 · 보상 수령 완료"):
   State.status()==Protocol::QUEST_READY_TO_TURN_IN?TEXT("목표 달성 · NPC에게 완료 보고하세요."):TEXT("진행 중"));
  View.RewardItemId=State.reward_item_id();View.RewardCount=State.reward_count();
 }
 Journal->SetQuest(View,bReady);
}
bool US1QuestSubsystem::CanTurnIn(uint32 Npc) const
{
 return bSupported&&bRewardSupported&&bReady&&State.status()==Protocol::QUEST_READY_TO_TURN_IN&&!State.rewarded()&&
  State.return_npc_id()==Npc&&State.accepted_instance_id().size()==16&&(PendingTurnIn.request_id().empty()||bTurnInUnconfirmed);
}
bool US1QuestSubsystem::RequestTurnIn(uint32 Npc,TFunction<void()> Committed)
{
 if(!CanTurnIn(Npc))return false;auto* N=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();if(!N||!N->IsConnected())return false;
 if(PendingTurnIn.request_id().empty())
 {
  PendingTurnIn.set_request_id(NewRequest());PendingTurnIn.set_quest_id(State.quest_id());PendingTurnIn.set_accepted_instance_id(State.accepted_instance_id());
  PendingTurnIn.set_npc_id(Npc);PendingTurnIn.set_contract_version(1);
 }
 else if(PendingTurnIn.npc_id()!=Npc||PendingTurnIn.accepted_instance_id()!=State.accepted_instance_id())return false;
 OnRewardCommitted=MoveTemp(Committed);TurnInAt=FPlatformTime::Seconds();bTurnInUnconfirmed=false;
 StatusText=TEXT("완료 보고·보상 저장 확인 중");N->SendPacket(ClientPacketHandler::MakeSendBuffer(PendingTurnIn));return true;
}
void US1QuestSubsystem::HandleTurnIn(const Protocol::S_QUEST_TURN_IN_RESULT& Packet)
{
 if(!bRewardSupported||!Matches(Packet.character_id(),Packet.contract_version(),Packet.quest_session_id())||
  !Protocol::QuestTurnInDecision_IsValid(Packet.decision())||!Protocol::QuestTurnInReason_IsValid(Packet.reason()))return;
 const bool bCommitted=Packet.decision()==Protocol::QUEST_TURN_IN_COMMITTED;
 if(bCommitted&&(Packet.reason()!=Protocol::QUEST_TURN_IN_REASON_NONE||!Packet.has_state()||!Packet.state().rewarded()||!Packet.inventory_version()))return;
 const bool bNewlyPaid=!State.rewarded()&&Packet.has_state()&&Packet.state().rewarded();
 if(Packet.has_state()&&AcceptSnapshot(State,Packet.state())){State=Packet.state();bReady=true;}
 if(bCommitted||bNewlyPaid)
 {if(auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>())Inventory->RefreshAfterQuestReward(Packet.inventory_version());}
 if(PendingTurnIn.request_id().empty()||PendingTurnIn.request_id()!=Packet.request_id())return;
 UE_LOG(LogTemp,Log,TEXT("Quest reward result. Request[%s] Decision[%d] Reason[%d] Inventory[%llu] Revision[%llu]"),
  *FString::FromHexBlob(reinterpret_cast<const uint8*>(Packet.request_id().data()),static_cast<uint32>(Packet.request_id().size())),static_cast<int32>(Packet.decision()),static_cast<int32>(Packet.reason()),Packet.inventory_version(),State.revision());
 if(bCommitted&&State.rewarded())
 {PendingTurnIn.Clear();bTurnInUnconfirmed=false;StatusText=TEXT("퀘스트 보상 수령 완료");auto Callback=MoveTemp(OnRewardCommitted);OnRewardCommitted=nullptr;if(Callback)Callback();return;}
 if(Packet.decision()==Protocol::QUEST_TURN_IN_REJECTED)
 {
  PendingTurnIn.Clear();OnRewardCommitted=nullptr;bTurnInUnconfirmed=false;
  StatusText=Packet.reason()==Protocol::QUEST_TURN_IN_INVENTORY_FULL?TEXT("인벤토리 공간 부족 · 공간 확보 후 다시 보고해 주세요"):
   Packet.reason()==Protocol::QUEST_TURN_IN_ALREADY_REWARDED?TEXT("이미 보상을 수령했습니다"):
   Packet.reason()==Protocol::QUEST_TURN_IN_BUSY?TEXT("다른 아이템 처리 중 · 잠시 후 다시 보고해 주세요"):
   FString::Printf(TEXT("보고되지 않음 (사유 %d)"),static_cast<int32>(Packet.reason()));return;
 }
 if(Packet.decision()==Protocol::QUEST_TURN_IN_UNCONFIRMED)
 {bTurnInUnconfirmed=true;StatusText=TEXT("보상 결과 확인 중 · 다시 누르면 동일 요청 재확인");Query();}
}
