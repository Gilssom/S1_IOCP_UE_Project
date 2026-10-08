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
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWidget.h"
#include "Loot/S1QuickSlotInputGate.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SViewport.h"
#include "UObject/ConstructorHelpers.h"

US1InventorySubsystem::US1InventorySubsystem()
{
 static ConstructorHelpers::FClassFinder<US1QuickSlotWidget> WidgetAsset(TEXT("/Game/S1/UI/WBP_S1QuickSlots"));
 QuickSlotWidgetClass=WidgetAsset.Class;
 static ConstructorHelpers::FClassFinder<US1InventoryWidget> InventoryAsset(TEXT("/Game/S1/UI/WBP_S1Inventory"));
 InventoryWidgetClass=InventoryAsset.Class;
}

void US1InventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
 Super::Initialize(Collection);Collection.InitializeDependency<US1NetworkSubsystem>();
 auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
 Network->OnItemPickupResultPacket.AddUObject(this,&US1InventorySubsystem::HandlePickup);
 Network->OnInventorySnapshotPacket.AddUObject(this,&US1InventorySubsystem::HandleInventory);
 Network->OnItemUseResultPacket.AddUObject(this,&US1InventorySubsystem::HandleUse);
 TickHandle=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&US1InventorySubsystem::Tick),0.1f);
}
void US1InventorySubsystem::Deinitialize()
{
 ResetSession();FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
 if(auto* Network=GetGameInstance()->GetSubsystem<US1NetworkSubsystem>())
 {Network->OnItemPickupResultPacket.RemoveAll(this);Network->OnInventorySnapshotPacket.RemoveAll(this);Network->OnItemUseResultPacket.RemoveAll(this);}
 Super::Deinitialize();
}
void US1InventorySubsystem::BeginSession(uint64 Character,bool bSupported,bool bUseSupported)
{ResetSession();CharacterId=Character;bEnabled=bSupported&&Character!=0;bUseEnabled=bEnabled&&bUseSupported;State.Reset(Character);Status=TEXT("서버 인벤토리 조회 대기");LoadQuickSlots();}
void US1InventorySubsystem::ResetSession()
{
 CloseInventory();CharacterId=0;bEnabled=false;bInitialQuery=false;bQueryPending=false;
 if(QuickWidget){QuickWidget->RemoveFromParent();QuickWidget=nullptr;}
 QuickSlots.Reset();QuickIdentity.Reset();
 bPickupUnconfirmed=false;QueryRequest.clear();PickupRequest.clear();PickupDrop.clear();PendingKey.Reset();
 State.Reset();Status.Reset();
 UseRequest.clear();UseItem=0;bUseEnabled=false;bUseUnconfirmed=false;
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
  if(Drop.OwnerCharacterId!=CharacterId||FMath::Abs(Foot-Drop.Floor.Z)>100.)continue;
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
 if(!bEnabled)return true;
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
 if(!Flow||!Flow->IsMainLevelReady()||!LocalPlayer())
 {if(QuickWidget){QuickWidget->RemoveFromParent();QuickWidget=nullptr;}return true;}
 auto* LocalPC=Cast<APlayerController>(LocalPlayer()->GetController());
 if(QuickWidget&&QuickWidget->GetWorld()!=GetWorld()){QuickWidget->RemoveFromParent();QuickWidget=nullptr;}
 if(!QuickWidget&&LocalPC&&LocalPC->IsLocalController())
 {
  QuickWidget=CreateWidget<US1QuickSlotWidget>(LocalPC,QuickSlotWidgetClass.Get()?QuickSlotWidgetClass.Get():US1QuickSlotWidget::StaticClass());
  if(QuickWidget){QuickWidget->SetIsFocusable(false);QuickWidget->SetVisibility(ESlateVisibility::HitTestInvisible);QuickWidget->AddToViewport(10);}
 }
 RefreshQuickView();
 if(!bInitialQuery){bInitialQuery=true;QueryInventory();}
 const double Now=FPlatformTime::Seconds();
 if(bQueryPending&&Now-QueryAt>5.)
 {bQueryPending=false;Status=TEXT("조회 응답 지연: I로 다시 조회");RefreshView();}
 if(!PickupRequest.empty()&&!bPickupUnconfirmed&&Now-PickupAt>5.)
 {bPickupUnconfirmed=true;Status=TEXT("획득 결과 미확정: G로 같은 요청 재확인");RefreshView();}
 if(!UseRequest.empty()&&!bUseUnconfirmed&&Now-UseAt>5.)
 {bUseUnconfirmed=true;Status=TEXT("사용 결과 미확정: 원래 요청 재확인 가능");RefreshView();}
 if(auto* Entities=GetWorld()->GetSubsystem<US1WorldEntitySubsystem>())
  Entities->UpdateLootHints(CharacterId,PickupRequest.empty()?SelectDrop():PendingKey,!PickupRequest.empty());
 return true;
}
void US1InventorySubsystem::RequestPickup()
{
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!bEnabled||!PC||PC->bShowMouseCursor||IsInventoryOpen()||PC->IsPaused()||
  Player->GetCharacterMovement()->MovementMode==MOVE_None)return;
 if(!PickupRequest.empty())
 {if(!bPickupUnconfirmed||FPlatformTime::Seconds()-PickupAt<1.)return;}
 else
 {
  PendingKey=SelectDrop();if(PendingKey.IsEmpty()||!DropBytes(PendingKey,PickupDrop))return;
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
 if(State.Apply(Packet))Status=bUseEnabled?TEXT("서버 확정 수량: 스택 선택 후 사용"):TEXT("서버 확정 수량 (사용 계약 미지원)");
 else Status=Packet.success()?TEXT("과거 또는 잘못된 목록 무시: I로 다시 조회"):TEXT("서버 조회 실패: I로 다시 조회");
 RefreshView();
 UE_LOG(LogTemp,Log,TEXT("Inventory snapshot. Success[%d] Version[%llu] DisplayCount[%lld]"),Packet.success(),Packet.inventory_version(),State.GetCount());
}
void US1InventorySubsystem::ToggleInventory()
{
 if(IsInventoryOpen()){CloseInventory();return;}
 auto* Flow=GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();auto* Player=LocalPlayer();
 auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(!bEnabled||!Flow||!Flow->IsMainLevelReady()||!PC||PC->IsPaused())return;
 InventoryWidget=CreateWidget<US1InventoryWidget>(PC,InventoryWidgetClass.Get()?InventoryWidgetClass.Get():US1InventoryWidget::StaticClass());
 if(!InventoryWidget)return;
 InventoryWidget->SetIsFocusable(true);
 InventoryWidget->OnCloseRequested.BindUObject(this,&US1InventorySubsystem::CloseInventory);
 InventoryWidget->OnUseRequested.BindUObject(this,&US1InventorySubsystem::RequestUseItem);
 InventoryWidget->OnRegisterRequested.BindUObject(this,&US1InventorySubsystem::RegisterQuickSlot);
 InventoryWidget->OnClearRequested.BindUObject(this,&US1InventorySubsystem::ClearQuickSlot);
 InventoryWidget->AddToViewport(50);PC->bShowMouseCursor=true;
 FInputModeGameAndUI Mode;Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
 Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());
 Mode.SetHideCursorDuringCapture(false);PC->SetInputMode(Mode);
 Player->GetCharacterMovement()->StopMovementImmediately();QueryInventory();RefreshView();
}
void US1InventorySubsystem::CloseInventory()
{
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
 InventoryWidget->SetUseView(Entries,bUseEnabled&&(bPending?bUseUnconfirmed:State.ReadyForUse()&&!bQueryPending&&PickupRequest.empty()),bPending&&bUseUnconfirmed);
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
 if(!IsInventoryOpen()||!bUseEnabled||!State.ReadyForUse()||bQueryPending||!UseRequest.empty()||!PickupRequest.empty())return;
 if(!State.GetEntries().ContainsByPredicate([ItemId](const auto& Entry){return Entry.item_instance_id()==ItemId&&Entry.item_definition_id()==20001&&Entry.count()>0;}))return;
 if(QuickSlots.Set(Slot,20001)){Status=FString::Printf(TEXT("퀵슬롯 %d 등록"),Slot+1);SaveQuickSlots();RefreshView();}
}
void US1InventorySubsystem::ClearQuickSlot(int32 Slot)
{
 if(!IsInventoryOpen())return;
 if(QuickSlots.Set(Slot,0)){Status=FString::Printf(TEXT("퀵슬롯 %d 해제"),Slot+1);SaveQuickSlots();RefreshView();}
}
bool US1InventorySubsystem::CanUseQuickSlot() const
{
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
