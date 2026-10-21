#include "Loot/S1InventorySubsystem.h"
#include "Loot/S1LootState.h"
#include "Level/S1LevelFlowSubsystem.h"
#include "Network/S1NetworkSubsystem.h"
#include "World/S1WorldEntitySubsystem.h"
#include "UI/S1InventoryWidget.h"
#include "Game/S1MyPlayer.h"
#include "ClientPacketHandler.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Account/S1AccountSubsystem.h"
#include "Loot/S1QuickSlotSaveGame.h"
#include "UI/S1QuickSlotWidget.h"
#include "UI/S1InventorySplitWidget.h"
#include "UI/S1InventoryDiscardWidget.h"
#include "UI/S1InventoryDropZoneWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWidget.h"
#include "Loot/S1QuickSlotInputGate.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SViewport.h"
#include "UObject/ConstructorHelpers.h"
#include "Quest/S1DialogueSubsystem.h"

namespace
{
 bool DialogueBlocks(const UWorld* World)
 {const auto* Dialogue=World?World->GetSubsystem<US1DialogueSubsystem>():nullptr;return Dialogue&&Dialogue->IsModal();}
}

US1InventorySubsystem::US1InventorySubsystem()
{
 // Match the existing inventory/quick-slot hard cooker references: a C++ soft path alone
 // was absent from the packaged registry despite loading successfully in editor tests.
 static ConstructorHelpers::FClassFinder<US1QuickSlotWidget> MainHUDAsset(TEXT("/Game/S1/UI/WBP_S1MainHUD"));
 MainHUDClass=MainHUDAsset.Class;
 static ConstructorHelpers::FClassFinder<US1QuickSlotWidget> WidgetAsset(TEXT("/Game/S1/UI/WBP_S1QuickSlots"));
 QuickSlotWidgetClass=WidgetAsset.Class;
 static ConstructorHelpers::FClassFinder<US1InventoryWidget> InventoryAsset(TEXT("/Game/S1/UI/WBP_S1InventoryGrid"));
 InventoryWidgetClass=InventoryAsset.Class;
 // The asset now exists. Hard native reference mirrors the grid/quick-slot cooker contract.
 static ConstructorHelpers::FClassFinder<US1InventorySplitWidget> SplitAsset(TEXT("/Game/S1/UI/WBP_S1InventorySplitDialog"));
 SplitWidgetClass=SplitAsset.Class;
 // Created and selectively saved before this hard cooker reference was introduced.
 static ConstructorHelpers::FClassFinder<US1InventoryDiscardWidget> DiscardAsset(TEXT("/Game/S1/UI/WBP_S1InventoryDiscardDialog"));
 DiscardWidgetClass=DiscardAsset.Class;
}

void US1InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
 Super::Initialize(Collection);Collection.InitializeDependency<US1NetworkSubsystem>();
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 Network->OnItemPickupResultPacket.AddUObject(this,&US1InventorySubsystem::HandlePickup);
 Network->OnInventorySnapshotPacket.AddUObject(this,&US1InventorySubsystem::HandleInventory);
 Network->OnItemUseResultPacket.AddUObject(this,&US1InventorySubsystem::HandleUse);
 Network->OnInventoryMoveResultPacket.AddUObject(this,&US1InventorySubsystem::HandleMove);
 Network->OnInventoryStackResultPacket.AddUObject(this,&US1InventorySubsystem::HandleStack);
 Network->OnPlayerFieldDropResultPacket.AddUObject(this,&US1InventorySubsystem::HandleDiscard);
 TickHandle=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&US1InventorySubsystem::Tick),0.1f);
}
void US1InventorySubsystem::Deinitialize()
{
 ResetSession();FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
 if(auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>())
 {Network->OnItemPickupResultPacket.RemoveAll(this);Network->OnInventorySnapshotPacket.RemoveAll(this);Network->OnItemUseResultPacket.RemoveAll(this);Network->OnInventoryMoveResultPacket.RemoveAll(this);Network->OnInventoryStackResultPacket.RemoveAll(this);}
 if(auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>())Network->OnPlayerFieldDropResultPacket.RemoveAll(this);
 Super::Deinitialize();
}
void US1InventorySubsystem::BeginSession(uint64 Character,bool bSupported,bool bUseSupported,bool bMoveSupported,bool bStackSupported,bool bDiscardSupported)
{ResetSession();CharacterId=Character;bEnabled=bSupported&&Character!=0;bUseEnabled=bEnabled&&bUseSupported;bMoveEnabled=bEnabled&&bMoveSupported;bStackEnabled=bEnabled&&bStackSupported;bDiscardEnabled=bEnabled&&bDiscardSupported;State.Reset(Character);Status=TEXT("서버 인벤토리 조회 대기");LoadQuickSlots();}
void US1InventorySubsystem::ResetSession()
{
 CloseInventory();CharacterId=0;bEnabled=false;bInitialQuery=false;bQueryPending=false;
 if(QuickWidget){QuickWidget->RemoveFromParent();QuickWidget=nullptr;}
 QuickSlots.Reset();QuickIdentity.Reset();
 bPickupUnconfirmed=false;QueryRequest.clear();PickupRequest.clear();PickupDrop.clear();PendingKey.Reset();
 State.Reset();Status.Reset();
 UseRequest.clear();UseItem=0;bUseEnabled=false;bUseUnconfirmed=false;
 MoveRequest.clear();MoveItem=0;MoveVersion=0;bMoveEnabled=false;bMoveReconciling=false;
 StackRequest.clear();StackItem=0;StackVersion=0;bStackEnabled=false;bStackReconciling=false;
 DiscardRequest.clear();DiscardItem=0;DiscardVersion=0;bDiscardEnabled=false;bDiscardReconciling=false;
}
void US1InventorySubsystem::RefreshAfterQuestReward(uint64 MinimumVersion)
{
 if(!bEnabled)return;
 State.RequireVersion(MinimumVersion);bQueryPending=false;QueryInventory();
}
AS1MyPlayer* US1InventorySubsystem::LocalPlayer() const
{
 auto* World=GetWorld();auto* PC=World?World->GetFirstPlayerController():nullptr;
 return PC?Cast<AS1MyPlayer>(PC->GetPawn()):nullptr;
}
std::string US1InventorySubsystem::NewRequest()
{
 const FGuid Guid=FGuid::NewGuid();std::string Result(16,'\0');
 const uint32 Parts[4]={Guid.A,Guid.B,Guid.C,Guid.D};
 for(int32 P=0;P<4;++P)for(int32 B=0;B<4;++B)Result[P*4+B]=static_cast<char>((Parts[P]>>(B*8))&255);
 return Result;
}
bool US1InventorySubsystem::DropBytes(const FString& Key,std::string& Bytes)
{
 if(Key.Len()!=32)return false;Bytes.clear();
 for(int32 I=0;I<32;I+=2)
 {if(!FChar::IsHexDigit(Key[I])||!FChar::IsHexDigit(Key[I+1]))return false;
  Bytes.push_back(static_cast<char>((FParse::HexDigit(Key[I])<<4)|FParse::HexDigit(Key[I+1])));}
 return Bytes.find_first_not_of('\0')!=std::string::npos;
}
FString US1InventorySubsystem::SelectDrop() const
{
 auto* Player=LocalPlayer();auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 if(!bEnabled||!Player||!Flow||!Flow->IsMainLevelReady()||IsInventoryOpen()||
  !Cast<APlayerController>(Player->GetController())||Cast<APlayerController>(Player->GetController())->bShowMouseCursor||
  Player->GetCharacterMovement()->MovementMode==MOVE_None)return {};
 const FVector Location=Player->GetActorLocation();
 const double Foot=Location.Z-Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
 FString Best;double Distance=150.*150.;
 for(const auto& Drop:Flow->GetLootState().GetDrops())
 {
  if((Drop.SourceKind!=2&&Drop.OwnerCharacterId!=CharacterId)||FMath::Abs(Foot-Drop.Floor.Z)>100.)continue;
  const double Candidate=FVector::DistSquaredXY(Location,Drop.Floor);
  if(Candidate>Distance||(Candidate==Distance&&!Best.IsEmpty()&&Drop.Key>=Best))continue;
  FCollisionQueryParams Params(SCENE_QUERY_STAT(S1PickupHint),false,Player);FHitResult Hit;
  if(GetWorld()->LineTraceSingleByChannel(Hit,FVector(Location.X,Location.Y,Foot+50.),Drop.Floor+FVector(0,0,50),ECC_Visibility,Params))continue;
  Best=Drop.Key;Distance=Candidate;
 }
 return Best;
}
bool US1InventorySubsystem::Tick(float Delta)
{
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 if(!Flow||!Flow->IsMainLevelReady()||!LocalPlayer())
 {if(QuickWidget){QuickWidget->RemoveFromParent();QuickWidget=nullptr;}return true;}
 auto* LocalPC=Cast<APlayerController>(LocalPlayer()->GetController());
 if(QuickWidget&&QuickWidget->GetWorld()!=GetWorld()){QuickWidget->RemoveFromParent();QuickWidget=nullptr;}
 if(!QuickWidget&&LocalPC&&LocalPC->IsLocalController())
 {
  auto* HUD=MainHUDClass.Get();
  QuickWidget=CreateWidget<US1QuickSlotWidget>(LocalPC,HUD?HUD:QuickSlotWidgetClass.Get()?QuickSlotWidgetClass.Get():US1QuickSlotWidget::StaticClass());
  if(QuickWidget){QuickWidget->SetIsFocusable(false);QuickWidget->SetVisibility(ESlateVisibility::HitTestInvisible);QuickWidget->AddToViewport(10);}
 }
 RefreshQuickView();
 // HP/combat HUD remains available even if the inventory capability is unavailable.
 // Economic queries and requests still require the negotiated inventory capability.
 if(!bEnabled)return true;
 if(!bInitialQuery){bInitialQuery=true;QueryInventory();}
 const double Now=FPlatformTime::Seconds();
 if(!DiscardRequest.empty()&&Now-DiscardAt>5.&&!bDiscardReconciling)
 {bDiscardReconciling=true;bQueryPending=false;Status=TEXT("버리기 결과 미확정: 수량 재조회 (재전송 없음)");QueryInventory();}
 if(!StackRequest.empty()&&Now-StackAt>5.&&!bStackReconciling)
 {bStackReconciling=true;bQueryPending=false;QueryInventory();}
 if(!MoveRequest.empty()&&Now-MoveAt>5.&&!bMoveReconciling)
 {bMoveReconciling=true;bQueryPending=false;Status=TEXT("이동 응답 지연: 서버 위치 재조회 (재전송 없음)");QueryInventory();}
 if(bQueryPending&&Now-QueryAt>5.)
 {bQueryPending=false;Status=TEXT("조회 응답 지연: I로 다시 조회");RefreshView();}
 if(!PickupRequest.empty()&&!bPickupUnconfirmed&&Now-PickupAt>5.)
 {bPickupUnconfirmed=true;Status=TEXT("획득 결과 미확정: G로 같은 요청 재확인");RefreshView();}
 if(!UseRequest.empty()&&!bUseUnconfirmed&&Now-UseAt>5.)
 {bUseUnconfirmed=true;Status=TEXT("사용 결과 미확정: 원래 요청 재확인 가능");RefreshView();}
 return true;
}
FString US1InventorySubsystem::GetPickupCandidate(float& DistanceSquared) const
{
 DistanceSquared=MAX_flt;
 const FString Key=SelectDrop();
 if(auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>())
  for(const auto& Drop:Flow->GetLootState().GetDrops())if(Drop.Key==Key)
   if(auto* Player=LocalPlayer())DistanceSquared=FVector::DistSquaredXY(Player->GetActorLocation(),Drop.Floor);
 return Key;
}
void US1InventorySubsystem::UpdateInteractionHint(const FString& VisibleKey,bool bHide)
{
 if(auto* World=GetWorld())if(auto* Entities=World->GetSubsystem<US1WorldEntitySubsystem>())
  Entities->UpdateLootHints(CharacterId,bHide?FString():PickupRequest.empty()?VisibleKey:PendingKey,!PickupRequest.empty());
}
void US1InventorySubsystem::RequestPickup(const FString& ExpectedDrop)
{
 if(DialogueBlocks(GetWorld()))return;
 if(HasDiscardWork())return;
 if(!StackRequest.empty()||SplitWidget)return;
 if(!MoveRequest.empty())return;
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!bEnabled||!PC||PC->bShowMouseCursor||IsInventoryOpen()||PC->IsPaused()||
  Player->GetCharacterMovement()->MovementMode==MOVE_None)return;
 if(!PickupRequest.empty())
 {if(!bPickupUnconfirmed||FPlatformTime::Seconds()-PickupAt<1.)return;}
 else
 {
  PendingKey=SelectDrop();if((!ExpectedDrop.IsEmpty()&&ExpectedDrop!=PendingKey)||PendingKey.IsEmpty()||!DropBytes(PendingKey,PickupDrop))return;
  PickupRequest=NewRequest();
 }
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();if(!Network||!Network->IsConnected())return;
 Protocol::C_ITEM_PICKUP Packet;Packet.set_contract_version(1);Packet.set_drop_id(PickupDrop);Packet.set_request_id(PickupRequest);
 bPickupUnconfirmed=false;PickupAt=FPlatformTime::Seconds();Status=TEXT("획득 처리 중");
 Network->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Pickup requested. Drop[%s] Request[%s]"),*PendingKey,*FString::FromHexBlob(reinterpret_cast<const uint8*>(PickupRequest.data()),16));
}
void US1InventorySubsystem::QueryInventory()
{
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 if(!bEnabled||bQueryPending||!Network||!Network->IsConnected())return;
 QueryRequest=NewRequest();bQueryPending=true;QueryAt=FPlatformTime::Seconds();Status=TEXT("서버 인벤토리 조회 중");
 Protocol::C_INVENTORY_QUERY Packet;Packet.set_contract_version(1);Packet.set_request_id(QueryRequest);
 Network->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));RefreshView();
}
void US1InventorySubsystem::HandlePickup(const Protocol::S_ITEM_PICKUP_RESULT& Packet)
{
 if(!bEnabled||PickupRequest.empty()||Packet.request_id()!=PickupRequest||Packet.drop_id()!=PickupDrop||
  !Protocol::ItemPickupReason_IsValid(Packet.reason()))return;
 if(Packet.decision()==Protocol::ITEM_PICKUP_COMMITTED)
 {
  if(Packet.reason()!=Protocol::ITEM_REASON_NONE||!Packet.item_instance_id()||!Packet.inventory_version()||
   Packet.resulting_count()==0||Packet.resulting_count()>99)return;
  State.RequireVersion(Packet.inventory_version());Status=TEXT("획득 확정: 인벤토리 조회 중");
 }
 else if(Packet.decision()==Protocol::ITEM_PICKUP_REJECTED)
 {if(Packet.reason()==Protocol::ITEM_REASON_NONE)return;Status=FString::Printf(TEXT("획득 거부 (사유 %d)"),static_cast<int32>(Packet.reason()));}
 else if(Packet.decision()==Protocol::ITEM_PICKUP_UNCONFIRMED)
 {bPickupUnconfirmed=true;Status=TEXT("획득 결과 미확정: G로 같은 요청 재확인");RefreshView();return;}
 else return;
 UE_LOG(LogTemp,Log,TEXT("Pickup result. Drop[%s] Decision[%d] Reason[%d] Version[%llu]"),*PendingKey,
  static_cast<int32>(Packet.decision()),static_cast<int32>(Packet.reason()),Packet.inventory_version());
 const bool bCommitted=Packet.decision()==Protocol::ITEM_PICKUP_COMMITTED;
 PickupRequest.clear();PickupDrop.clear();PendingKey.Reset();bPickupUnconfirmed=false;RefreshView();
 if(bCommitted){bQueryPending=false;QueryInventory();} // Supersede any pre-commit read request.
}
void US1InventorySubsystem::HandleInventory(const Protocol::S_INVENTORY_SNAPSHOT& Packet)
{
 if(!bEnabled||!bQueryPending||Packet.request_id()!=QueryRequest||Packet.character_id()!=CharacterId)return;
 bQueryPending=false;
 if(State.Apply(Packet))
 {
  if(bMoveReconciling){MoveRequest.clear();MoveItem=0;bMoveReconciling=false;}
  if(bStackReconciling){StackRequest.clear();StackItem=0;bStackReconciling=false;}
  if(bDiscardReconciling){DiscardRequest.clear();DiscardItem=0;bDiscardReconciling=false;}
  if(DiscardWidget&&State.GetVersion()!=DiscardVersion)CancelDiscard();
  if(SplitWidget&&State.GetVersion()!=SplitVersion)CancelSplit();
  Status=bUseEnabled?TEXT("서버 확정 수량: 스택 선택 후 사용"):TEXT("서버 확정 수량 (사용 계약 미지원)");
 }
 else Status=Packet.success()?TEXT("과거 또는 잘못된 목록 무시: I로 다시 조회"):TEXT("서버 조회 실패: I로 다시 조회");
 RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Inventory snapshot. Success[%d] Version[%llu] DisplayCount[%lld]"),Packet.success(),Packet.inventory_version(),State.GetCount());
}
void US1InventorySubsystem::ToggleInventory()
{
 if(DialogueBlocks(GetWorld()))return;
 if(IsInventoryOpen()){CloseInventory();return;}
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();auto* Player=LocalPlayer();
 auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!bEnabled||!Flow||!Flow->IsMainLevelReady()||!PC||PC->IsPaused())return;
 if(!InventoryWidgetClass)InventoryWidgetClass=LoadClass<US1InventoryWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1InventoryGrid.WBP_S1InventoryGrid_C"));
 InventoryWidget=CreateWidget<US1InventoryWidget>(PC,InventoryWidgetClass.Get()?InventoryWidgetClass.Get():US1InventoryWidget::StaticClass());
 if(!InventoryWidget)return;
 InventoryWidget->SetIsFocusable(true);
 InventoryWidget->OnCloseRequested.BindUObject(this,&US1InventorySubsystem::CloseInventory);
 InventoryWidget->OnUseRequested.BindUObject(this,&US1InventorySubsystem::RequestUseItem);
 InventoryWidget->OnRegisterRequested.BindUObject(this,&US1InventorySubsystem::RegisterQuickSlot);
 InventoryWidget->OnClearRequested.BindUObject(this,&US1InventorySubsystem::ClearQuickSlot);
 InventoryWidget->AddToViewport(50);PC->bShowMouseCursor=true;
 DropZoneWidget=CreateWidget<US1InventoryDropZoneWidget>(PC,US1InventoryDropZoneWidget::StaticClass());
 if(DropZoneWidget){DropZoneWidget->SetIsFocusable(false);DropZoneWidget->AddToViewport(1);}
 FInputModeGameAndUI Mode;Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
 Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());
 Mode.SetHideCursorDuringCapture(false);PC->SetInputMode(Mode);
 Player->GetCharacterMovement()->StopMovementImmediately();QueryInventory();RefreshView();
}
void US1InventorySubsystem::CloseInventory()
{
 CancelDiscard();
 if(DropZoneWidget){DropZoneWidget->RemoveFromParent();DropZoneWidget=nullptr;}
 CancelSplit();
 if(!InventoryWidget)return;
 InventoryWidget->OnCloseRequested.Unbind();InventoryWidget->OnUseRequested.Unbind();
 InventoryWidget->OnRegisterRequested.Unbind();InventoryWidget->OnClearRequested.Unbind();
 InventoryWidget->RemoveFromParent();InventoryWidget=nullptr;
 if(auto* Player=LocalPlayer())if(auto* PC=Cast<APlayerController>(Player->GetController()))
 {PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());}
}
void US1InventorySubsystem::RefreshView()
{
 RefreshQuickView();
 if(!InventoryWidget)return;
 InventoryWidget->SetPotionView(State.GetCount(),Status);
 TArray<FS1InventoryEntryView> Entries;
 for(const auto& Entry:State.GetEntries())
  if(Entry.item_definition_id()==20001)Entries.Add({Entry.item_instance_id(),Entry.count(),Entry.slot_index()});
 const bool bPending=!UseRequest.empty();
 InventoryWidget->SetUseView(Entries,!HasDiscardWork()&&StackRequest.empty()&&!SplitWidget&&MoveRequest.empty()&&bUseEnabled&&(bPending?bUseUnconfirmed:State.ReadyForUse()&&!bQueryPending&&PickupRequest.empty()),bPending&&bUseUnconfirmed);
}

bool US1InventorySubsystem::RequestMoveItem(uint64 ItemId,uint32 SourceSlot,uint32 Destination,uint64 Version,US1InventoryWidget* Source)
{
 if(HasDiscardWork())return false;
 if(!StackRequest.empty()||SplitWidget)return false;
 if(!bMoveEnabled||!Source||Source!=InventoryWidget||SourceSlot>=20||Destination>=20||SourceSlot==Destination||
  !State.ReadyForUse()||Version!=State.GetVersion()||bQueryPending||!MoveRequest.empty()||!UseRequest.empty()||!PickupRequest.empty())return false;
 const auto& Entries=State.GetEntries();
 if(!Entries.ContainsByPredicate([&](const auto& E){return E.item_instance_id()==ItemId&&E.slot_index()==SourceSlot&&E.count()>0;})||
  Entries.ContainsByPredicate([&](const auto& E){return E.slot_index()==Destination;}))return false;
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!Network||!Network->IsConnected()||!Flow||!Flow->IsMainLevelReady()||!PC||PC->IsPaused())return false;
 MoveRequest=NewRequest();MoveItem=ItemId;MoveSource=SourceSlot;MoveDestination=Destination;MoveVersion=Version;
 bMoveReconciling=false;MoveAt=FPlatformTime::Seconds();
 Protocol::C_INVENTORY_MOVE Packet;Packet.set_request_id(MoveRequest);Packet.set_item_instance_id(ItemId);
 Packet.set_source_slot(SourceSlot);Packet.set_destination_slot(Destination);Packet.set_expected_inventory_version(Version);Packet.set_contract_version(1);
 Status=TEXT("슬롯 이동 저장 중");Network->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Inventory move requested. Item[%llu] From[%u] To[%u] Version[%llu] Request[%s]"),
  ItemId,SourceSlot,Destination,Version,*FString::FromHexBlob(reinterpret_cast<const uint8*>(MoveRequest.data()),16));
 return true;
}
void US1InventorySubsystem::HandleMove(const Protocol::S_INVENTORY_MOVE_RESULT& Packet)
{
 if(!bMoveEnabled||MoveRequest.empty()||Packet.request_id()!=MoveRequest||Packet.character_id()!=CharacterId||
  Packet.item_instance_id()!=MoveItem||Packet.source_slot()!=MoveSource||Packet.destination_slot()!=MoveDestination||
  Packet.decision()<1||Packet.decision()>3||Packet.reason()>7)return;
 if(Packet.decision()==1&&(Packet.reason()!=0||Packet.inventory_version()!=MoveVersion+1))return;
 if(Packet.decision()==2&&Packet.reason()==0)return;
 UE_LOG(LogTemp,Log,TEXT("Inventory move result. Item[%llu] From[%u] To[%u] Decision[%u] Reason[%u] Version[%llu] Request[%s]"),
  MoveItem,MoveSource,MoveDestination,Packet.decision(),Packet.reason(),Packet.inventory_version(),
  *FString::FromHexBlob(reinterpret_cast<const uint8*>(MoveRequest.data()),16));
 if(Packet.decision()==1)State.RequireVersion(Packet.inventory_version());
 // All outcomes reconcile by a fresh snapshot; an uncertain response is not a rejection.
 bMoveReconciling=true;bQueryPending=false;QueryInventory();
}

bool US1InventorySubsystem::ValidateStack(uint32 Action,uint64 Item,uint32 Source,uint32 Destination,uint64 Target,uint64 Version,uint32 Amount) const
{
 if(HasDiscardWork())return false;
 if(!bStackEnabled||!InventoryWidget||Source>=20||Destination>=20||Source==Destination||Action<1||Action>3||
  !State.ReadyForUse()||Version!=State.GetVersion()||bQueryPending||!StackRequest.empty()||!MoveRequest.empty()||!UseRequest.empty()||!PickupRequest.empty())return false;
 const auto& Entries=State.GetEntries();
 const auto* S=Entries.FindByPredicate([&](const auto& E){return E.item_instance_id()==Item&&E.slot_index()==Source;});
 const auto* T=Entries.FindByPredicate([&](const auto& E){return E.slot_index()==Destination;});
 if(!S||!S->count()||S->count()>99)return false;
 if(Action==3)return !T&&!Target&&S->item_definition_id()==20001&&Amount>0&&Amount<S->count();
 if(!T||T->item_instance_id()!=Target||!Target||Target==Item||!T->count()||T->count()>99||Amount!=0)return false;
 return Action==1||(S->item_definition_id()==20001&&T->item_definition_id()==20001&&T->count()<99);
}
bool US1InventorySubsystem::HandleInventoryDrop(uint64 Item,uint32 Source,uint32 Destination,uint64 Target,uint64 Version,US1InventoryWidget* Owner,bool bSplit,bool bSwap)
{
 if(HasDiscardWork())return false;
 if(!Owner||Owner!=InventoryWidget||SplitWidget)return false;
 if(!bSplit&&!Target)return RequestMoveItem(Item,Source,Destination,Version,Owner);
 if(!bStackEnabled){Status=TEXT("서버가 스택 조작을 지원하지 않습니다.");RefreshView();return false;}
 if(bSplit)
 {
  if(!ValidateStack(3,Item,Source,Destination,Target,Version,1))return false;
  auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
  if(!PC||PC->IsPaused())return false;
  const auto* E=State.GetEntries().FindByPredicate([&](const auto& V){return V.item_instance_id()==Item;});
  auto* Class=SplitWidgetClass.Get();
  SplitWidget=CreateWidget<US1InventorySplitWidget>(PC,Class?Class:US1InventorySplitWidget::StaticClass());
  if(!SplitWidget)return false;
  SplitItem=Item;SplitSource=Source;SplitDestination=Destination;SplitVersion=Version;
  SplitWidget->SetIsFocusable(true);SplitWidget->SetSourceCount(E->count());
  SplitWidget->OnConfirm.BindUObject(this,&US1InventorySubsystem::ConfirmSplit);SplitWidget->OnCancel.BindUObject(this,&US1InventorySubsystem::CancelSplit);
  SplitWidget->AddToViewport(70);
  FInputModeGameAndUI Mode;Mode.SetWidgetToFocus(SplitWidget->TakeWidget());Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
  PC->SetInputMode(Mode);PC->bShowMouseCursor=true;RefreshView();return true;
 }
 const auto* S=State.GetEntries().FindByPredicate([&](const auto& E){return E.item_instance_id()==Item;});
 const auto* T=State.GetEntries().FindByPredicate([&](const auto& E){return E.item_instance_id()==Target;});
 const uint32 Action=bSwap||(S&&T&&S->item_definition_id()!=T->item_definition_id())?1:2;
 if(Action==2&&T&&T->count()==99){Status=TEXT("목적지 스택이 가득 찼습니다. (최대 99개)");RefreshView();return false;}
 return RequestStack(Action,Item,Source,Destination,Target,Version,0);
}
void US1InventorySubsystem::CancelSplit()
{
 if(!SplitWidget)return;
 SplitWidget->OnConfirm.Unbind();SplitWidget->OnCancel.Unbind();SplitWidget->RemoveFromParent();SplitWidget=nullptr;SplitItem=0;
 if(InventoryWidget)
 {
  if(auto* Player=LocalPlayer())if(auto* PC=Cast<APlayerController>(Player->GetController()))
  {FInputModeGameAndUI Mode;Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);PC->SetInputMode(Mode);}
 }
 RefreshView();
}
void US1InventorySubsystem::ConfirmSplit(uint32 Amount)
{
 if(!SplitWidget)return;
 const auto Item=SplitItem,Version=SplitVersion;const auto Source=SplitSource,Destination=SplitDestination;
 CancelSplit();
 if(!RequestStack(3,Item,Source,Destination,0,Version,Amount))
 {Status=TEXT("분할할 상태가 변경되었습니다. 다시 확인해 주세요.");RefreshView();}
}
bool US1InventorySubsystem::RequestStack(uint32 Action,uint64 Item,uint32 Source,uint32 Destination,uint64 Target,uint64 Version,uint32 Amount)
{
 if(SplitWidget||!ValidateStack(Action,Item,Source,Destination,Target,Version,Amount))return false;
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!Network||!Network->IsConnected()||!Flow||!Flow->IsMainLevelReady()||!PC||PC->IsPaused())return false;
 StackRequest=NewRequest();StackItem=Item;StackVersion=Version;StackSource=Source;StackDestination=Destination;StackAction=Action;StackAt=FPlatformTime::Seconds();bStackReconciling=false;
 Protocol::C_INVENTORY_STACK_ACTION Packet;Packet.set_request_id(StackRequest);Packet.set_action(Action);Packet.set_source_item_id(Item);Packet.set_source_slot(Source);
 Packet.set_destination_slot(Destination);Packet.set_expected_destination_item_id(Target);Packet.set_expected_inventory_version(Version);Packet.set_amount(Amount);Packet.set_contract_version(1);
 Status=TEXT("스택 조작 저장 중");Network->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Inventory stack requested. Action[%u] Item[%llu] From[%u] To[%u] Amount[%u] Version[%llu] Request[%s]"),
 Action,Item,Source,Destination,Amount,Version,*FString::FromHexBlob(reinterpret_cast<const uint8*>(StackRequest.data()),16));return true;
}
void US1InventorySubsystem::HandleStack(const Protocol::S_INVENTORY_STACK_ACTION_RESULT& Packet)
{
 if(!bStackEnabled||StackRequest.empty()||Packet.request_id()!=StackRequest||Packet.character_id()!=CharacterId||Packet.action()!=StackAction||
  Packet.source_item_id()!=StackItem||Packet.source_slot()!=StackSource||Packet.destination_slot()!=StackDestination||Packet.decision()<1||Packet.decision()>3||Packet.reason()>11)return;
 if(Packet.decision()==1&&(Packet.reason()!=0||Packet.inventory_version()!=StackVersion+1||Packet.source_count()>99||!Packet.destination_count()||Packet.destination_count()>99||
  (StackAction==3&&(!Packet.created_item_id()||Packet.created_item_id()==StackItem||!Packet.source_count()))))return;
 if(Packet.decision()==2&&Packet.reason()==0)return;
 UE_LOG(LogTemp,Log,TEXT("Inventory stack result. Action[%u] Item[%llu] Decision[%u] Reason[%u] Version[%llu] Request[%s]"),
 StackAction,StackItem,Packet.decision(),Packet.reason(),Packet.inventory_version(),*FString::FromHexBlob(reinterpret_cast<const uint8*>(StackRequest.data()),16));
 if(Packet.decision()==1)State.RequireVersion(Packet.inventory_version());
 bStackReconciling=true;bQueryPending=false;QueryInventory();
}

bool US1InventorySubsystem::ValidateDiscard(uint64 Item,uint32 Source,uint64 Version,uint32 Amount) const
{
 if(!bDiscardEnabled||!InventoryWidget||!Item||Source>=20||!Amount||Amount>99||!State.ReadyForUse()||Version!=State.GetVersion()||bQueryPending||
  !DiscardRequest.empty()||SplitWidget||!StackRequest.empty()||!MoveRequest.empty()||!UseRequest.empty()||!PickupRequest.empty())return false;
 const auto* E=State.GetEntries().FindByPredicate([&](const auto& V){return V.item_instance_id()==Item&&V.slot_index()==Source&&V.item_definition_id()==20001;});
 return E&&E->count()>0&&E->count()<=99&&Amount<=E->count();
}
bool US1InventorySubsystem::HandleDiscardDrop(uint64 Item,uint32 Source,uint64 Version,US1InventoryWidget* Owner)
{
 if(HasDiscardWork()||!Owner||Owner!=InventoryWidget||!ValidateDiscard(Item,Source,Version,1))return false;
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 if(!PC||PC->IsPaused()||!Network||!Network->IsConnected()||!Flow||!Flow->IsMainLevelReady())return false;
 const auto* E=State.GetEntries().FindByPredicate([&](const auto& V){return V.item_instance_id()==Item;});
 DiscardWidget=CreateWidget<US1InventoryDiscardWidget>(PC,DiscardWidgetClass.Get()?DiscardWidgetClass.Get():US1InventoryDiscardWidget::StaticClass());
 if(!DiscardWidget)return false;
 DiscardItem=Item;DiscardSource=Source;DiscardVersion=Version;DiscardOriginalCount=E->count();
 DiscardWidget->SetSourceCount(E->count());DiscardWidget->OnConfirm.BindUObject(this,&US1InventorySubsystem::ConfirmDiscard);DiscardWidget->OnCancel.BindUObject(this,&US1InventorySubsystem::CancelDiscard);
 DiscardWidget->AddToViewport(70);
 FInputModeGameAndUI Mode;Mode.SetWidgetToFocus(DiscardWidget->TakeWidget());Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);PC->SetInputMode(Mode);PC->bShowMouseCursor=true;
 DiscardWidget->FocusSafeDefault();
 RefreshView();return true;
}
void US1InventorySubsystem::CancelDiscard()
{
 if(!DiscardWidget)return;
 DiscardWidget->OnConfirm.Unbind();DiscardWidget->OnCancel.Unbind();DiscardWidget->RemoveFromParent();DiscardWidget=nullptr;
 if(InventoryWidget)if(auto* Player=LocalPlayer())if(auto* PC=Cast<APlayerController>(Player->GetController()))
 {FInputModeGameAndUI Mode;Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);PC->SetInputMode(Mode);}
 RefreshView();
}
void US1InventorySubsystem::ConfirmDiscard(uint32 Amount)
{
 if(!DiscardWidget)return;
 const auto Item=DiscardItem,Version=DiscardVersion;const auto Source=DiscardSource;
 CancelDiscard();
 if(!ValidateDiscard(Item,Source,Version,Amount)){Status=TEXT("버릴 아이템 상태가 변경되었습니다.");RefreshView();return;}
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!Network||!Network->IsConnected()||!Flow||!Flow->IsMainLevelReady()||!PC||PC->IsPaused())return;
 DiscardRequest=NewRequest();DiscardItem=Item;DiscardVersion=Version;DiscardSource=Source;DiscardAmount=Amount;DiscardAt=FPlatformTime::Seconds();bDiscardReconciling=false;
 Protocol::C_PLAYER_FIELD_DROP Packet;Packet.set_request_id(DiscardRequest);Packet.set_source_item_id(Item);Packet.set_source_slot(Source);Packet.set_amount(Amount);Packet.set_expected_inventory_version(Version);Packet.set_contract_version(1);
 Network->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));Status=TEXT("버리기 저장 중");RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Inventory discard requested. Item[%llu] Amount[%u] Version[%llu] Request[%s]"),Item,Amount,Version,*FString::FromHexBlob(reinterpret_cast<const uint8*>(DiscardRequest.data()),16));
}
void US1InventorySubsystem::HandleDiscard(const Protocol::S_PLAYER_FIELD_DROP_RESULT& Packet)
{
 if(!bDiscardEnabled||DiscardRequest.empty()||Packet.request_id()!=DiscardRequest||Packet.character_id()!=CharacterId||Packet.source_item_id()!=DiscardItem||Packet.source_slot()!=DiscardSource||Packet.requested_amount()!=DiscardAmount||Packet.decision()<1||Packet.decision()>3||Packet.reason()>10)return;
 FString DropKey;
 if(Packet.decision()==1&&!FS1LootState::KeyFromBytes(Packet.drop_id(),DropKey))return;
 if(Packet.decision()==1&&(Packet.reason()!=0||Packet.inventory_version()!=DiscardVersion+1||DiscardAmount>DiscardOriginalCount||Packet.remaining_count()!=DiscardOriginalCount-DiscardAmount))return;
 if(Packet.decision()==2&&Packet.reason()==0)return;
 UE_LOG(LogTemp,Log,TEXT("Inventory discard result. Item[%llu] Amount[%u] Decision[%u] Reason[%u] Version[%llu] Request[%s]"),DiscardItem,DiscardAmount,Packet.decision(),Packet.reason(),Packet.inventory_version(),*FString::FromHexBlob(reinterpret_cast<const uint8*>(DiscardRequest.data()),16));
 if(Packet.decision()==1)State.RequireVersion(Packet.inventory_version());
 bDiscardReconciling=true;bQueryPending=false;QueryInventory();
}

void US1InventorySubsystem::LoadQuickSlots()
{
 auto* Account=GetGameInstance()->GetSubsystem<US1AccountSubsystem>();
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 QuickIdentity=FS1QuickSlotState::Namespace(Network?Network->GetServerProfileKey():FString(),
  Account?Account->GetAuthenticatedAccountId():0,CharacterId);
 if(QuickIdentity.IsEmpty())return;
 const FString Name=TEXT("S1QuickSlots_")+QuickIdentity;
 if(!UGameplayStatics::DoesSaveGameExist(Name,0))return;
 if(auto* Saved=Cast<US1QuickSlotSaveGame>(UGameplayStatics::LoadGameFromSlot(Name,0)))
  QuickSlots.Restore(Saved->Version,Saved->IdentityHash,QuickIdentity,Saved->Definitions);
}
void US1InventorySubsystem::SaveQuickSlots()
{
 if(QuickIdentity.IsEmpty())return;
 auto* Saved=NewObject<US1QuickSlotSaveGame>();Saved->IdentityHash=QuickIdentity;Saved->Definitions=QuickSlots.Definitions;
 if(!UGameplayStatics::SaveGameToSlot(Saved,TEXT("S1QuickSlots_")+QuickIdentity,0))
 {Status=TEXT("슬롯 적용됨: 로컬 저장 실패");UE_LOG(LogTemp,Warning,TEXT("Quick-slot preference save failed; session assignments retained."));}
}
void US1InventorySubsystem::RegisterQuickSlot(int32 Slot,uint64 ItemId)
{
 if(HasDiscardWork())return;
 if(!StackRequest.empty()||SplitWidget)return;
 if(!MoveRequest.empty())return;
 if(!IsInventoryOpen()||!bUseEnabled||!State.ReadyForUse()||bQueryPending||!UseRequest.empty()||!PickupRequest.empty())return;
 if(!State.GetEntries().ContainsByPredicate([ItemId](const auto& Entry){return Entry.item_instance_id()==ItemId&&Entry.item_definition_id()==20001&&Entry.count()>0;}))return;
 if(QuickSlots.Set(Slot,20001)){Status=FString::Printf(TEXT("퀵슬롯 %d 등록"),Slot+1);SaveQuickSlots();RefreshView();}
}
void US1InventorySubsystem::ClearQuickSlot(int32 Slot)
{
 if(!IsInventoryOpen())
 {
  auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
  auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
  if(!bEnabled||!CharacterId||!PC||!PC->IsLocalController()||!PC->bShowMouseCursor||!Flow||!Flow->IsMainLevelReady()||PC->IsPaused())return;
 }
 if(QuickSlots.Set(Slot,0)){Status=FString::Printf(TEXT("퀵슬롯 %d 해제"),Slot+1);SaveQuickSlots();RefreshView();}
}
bool US1InventorySubsystem::RegisterQuickSlotDrag(int32 Slot,uint64 ItemId,US1InventoryWidget* Source)
{
 if(HasDiscardWork())return false;
 if(!StackRequest.empty()||SplitWidget)return false;
 if(!MoveRequest.empty())return false;
 if(!Source||Source!=InventoryWidget||Slot<0||Slot>=4||!bUseEnabled||!State.ReadyForUse()||
  bQueryPending||!UseRequest.empty()||!PickupRequest.empty())return false;
 if(!State.GetEntries().ContainsByPredicate([ItemId](const auto& E){return E.item_instance_id()==ItemId&&E.item_definition_id()==20001&&E.count()>0;}))return false;
 RegisterQuickSlot(Slot,ItemId);return true;
}
bool US1InventorySubsystem::CanUseQuickSlot() const
{
 if(DialogueBlocks(GetWorld()))return false;
 if(HasDiscardWork())return false;
 if(!StackRequest.empty()||SplitWidget)return false;
 if(!MoveRequest.empty())return false;
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 if(!bUseEnabled||!Flow||!Flow->IsMainLevelReady()||!Network||!Network->IsConnected()||!PC||!PC->IsLocalController()||
  PC->IsPaused()||PC->bShowMouseCursor||IsInventoryOpen()||Player->GetCharacterMovement()->MovementMode==MOVE_None||
  !UseRequest.empty()||!PickupRequest.empty()||bQueryPending||!State.ReadyForUse())return false;
 if(FSlateApplication::IsInitialized())
 {
  const auto& Slate=FSlateApplication::Get();
  const auto* Local=PC->GetLocalPlayer();
  const auto* ViewportClient=Local?Local->ViewportClient.Get():nullptr;
  const TSharedPtr<SWidget> Viewport=ViewportClient?ViewportClient->GetGameViewportWidget():nullptr;
  if(!FS1QuickSlotInputGate::AllowsFocus(Slate.GetKeyboardFocusedWidget(),Viewport,
   Slate.GetActiveModalWindow().IsValid()))return false;
 }
 return true;
}
void US1InventorySubsystem::RequestQuickSlot(int32 Slot)
{
 // Unknown is NOT an instruction to retry via another slot. Inventory retains explicit same-ID readback.
 const bool bAllowed=UseRequest.empty()&&CanUseQuickSlot();
 const auto Focus=FSlateApplication::IsInitialized()?FSlateApplication::Get().GetKeyboardFocusedWidget():nullptr;
 UE_LOG(LogTemp,Log,TEXT("Quick-slot input. Slot[%d] Allowed[%d] InventoryOpen[%d] SnapshotReady[%d] QueryPending[%d] UsePending[%d] Focus[%s]"),
  Slot+1,bAllowed,IsInventoryOpen(),State.ReadyForUse(),bQueryPending,!UseRequest.empty(),Focus.IsValid()?*Focus->GetTypeAsString():TEXT("None"));
 if(!bAllowed)return;
 if(const uint64 Item=QuickSlots.Resolve(Slot,State))RequestUseItem(Item);
}
void US1InventorySubsystem::RefreshQuickView()
{
 if(!QuickWidget)return;
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 QuickWidget->SetVisibility(DialogueBlocks(GetWorld())?ESlateVisibility::Collapsed:
  IsInventoryOpen()||(PC&&PC->bShowMouseCursor)?ESlateVisibility::SelfHitTestInvisible:ESlateVisibility::HitTestInvisible);
 FString Label;
 if(!bUseEnabled)Label=TEXT("미지원");
 else if(!UseRequest.empty())Label=bUseUnconfirmed?TEXT("미확정 (I)"):TEXT("처리 중");
 else if(!State.ReadyForUse()||bQueryPending)Label=TEXT("조회 중");
 else if(!PickupRequest.empty())Label=TEXT("획득 중");
 else if(State.GetCount()==0)Label=TEXT("보유 없음");
 else if(!CanUseQuickSlot())Label=TEXT("사용 대기");
 else Label=TEXT("사용 가능");
 QuickWidget->SetView(QuickSlots.Definitions,State.GetCount(),Label,CanUseQuickSlot());
}

void US1InventorySubsystem::RequestUseItem(uint64 ItemId)
{
 if(DialogueBlocks(GetWorld()))return;
 if(HasDiscardWork())return;
 if(!StackRequest.empty()||SplitWidget)return;
 if(!MoveRequest.empty())return;
 auto* Player=LocalPlayer();auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!bUseEnabled||!Flow||!Flow->IsMainLevelReady()||!Network||!Network->IsConnected()||!PC||PC->IsPaused()||
  Player->GetCharacterMovement()->MovementMode==MOVE_None)return;
 if(!UseRequest.empty())
 {if(!bUseUnconfirmed||FPlatformTime::Seconds()-UseAt<1.)return;}
 else
 {
  if(!State.ReadyForUse()||bQueryPending||!PickupRequest.empty()||!State.GetEntries().ContainsByPredicate([ItemId](const auto& Entry)
   {return Entry.item_instance_id()==ItemId&&Entry.item_definition_id()==20001&&Entry.count()>0;}))return;
  UseItem=ItemId;UseRequest=NewRequest();
 }
 Protocol::C_ITEM_USE Packet;Packet.set_contract_version(1);Packet.set_request_id(UseRequest);
 Packet.set_item_instance_id(UseItem);Packet.set_item_definition_id(20001);
 bUseUnconfirmed=false;UseAt=FPlatformTime::Seconds();Status=TEXT("포션 사용 처리 중");
 Network->SendPacket(ClientPacketHandler::MakeSendBuffer(Packet));RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Item use requested. Item[%llu] Request[%s]"),UseItem,
  *FString::FromHexBlob(reinterpret_cast<const uint8*>(UseRequest.data()),16));
}
void US1InventorySubsystem::HandleUse(const Protocol::S_ITEM_USE_RESULT& Packet)
{
 if(!bUseEnabled||UseRequest.empty()||Packet.request_id()!=UseRequest||Packet.character_id()!=CharacterId||
  Packet.item_instance_id()!=UseItem||Packet.item_definition_id()!=20001||!Protocol::ItemUseReason_IsValid(Packet.reason()))return;
 const auto Decision=Packet.decision();
 if(Decision==Protocol::ITEM_USE_APPLIED||Decision==Protocol::ITEM_USE_REFUNDED)
 {
  if(Packet.operation_id().size()!=16||Packet.operation_id().find_first_not_of('\0')==std::string::npos||
   !Packet.inventory_version()||Packet.resulting_count()>99)return;
  if(Decision==Protocol::ITEM_USE_APPLIED&&(Packet.reason()!=Protocol::ITEM_USE_REASON_NONE||!Packet.actual_heal()||Packet.actual_heal()>50))return;
  if(Decision==Protocol::ITEM_USE_REFUNDED&&(Packet.actual_heal()!=0||!Packet.resulting_count()||
   (Packet.reason()!=Protocol::ITEM_USE_REASON_DEAD&&Packet.reason()!=Protocol::ITEM_USE_REASON_FULL_HP&&Packet.reason()!=Protocol::ITEM_USE_REASON_SESSION_CHANGED)))return;
  State.RequireVersion(Packet.inventory_version());
  Status=Decision==Protocol::ITEM_USE_APPLIED?TEXT("사용 확정: 서버 수량 조회 중"):TEXT("사용 취소·반환 확정: 수량 조회 중");
 }
 else if(Decision==Protocol::ITEM_USE_REJECTED)
 {if(Packet.reason()==Protocol::ITEM_USE_REASON_NONE||Packet.actual_heal())return;
  Status=FString::Printf(TEXT("사용 거부 (사유 %d)"),static_cast<int32>(Packet.reason()));}
 else if(Decision==Protocol::ITEM_USE_UNCONFIRMED)
 {bUseUnconfirmed=true;Status=TEXT("사용 결과 미확정: 같은 요청 재확인");RefreshView();return;}
 else return;
 UE_LOG(LogTemp,Log,TEXT("Item use result. Item[%llu] Decision[%d] Reason[%d] Version[%llu] Heal[%u] Request[%s]"),
  UseItem,static_cast<int32>(Decision),static_cast<int32>(Packet.reason()),Packet.inventory_version(),Packet.actual_heal(),
  *FString::FromHexBlob(reinterpret_cast<const uint8*>(UseRequest.data()),16));
 UseRequest.clear();UseItem=0;bUseUnconfirmed=false;RefreshView();
 if(Decision!=Protocol::ITEM_USE_REJECTED){bQueryPending=false;QueryInventory();}
}
