#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Loot/S1InventoryState.h"
#include "Loot/S1QuickSlotState.h"
#include "S1InventorySubsystem.generated.h"

class US1InventoryWidget;
class US1QuickSlotWidget;
class US1InventorySplitWidget;
class US1InventoryDiscardWidget;
class US1InventoryDropZoneWidget;
class AS1MyPlayer;

/** Request coordination and inventory UI state, not world actor ownership or transport. */
UCLASS()
class S1_API US1InventorySubsystem : public UGameInstanceSubsystem
{
 GENERATED_BODY()
 friend class FS1PickupCoordinatorTest;
 friend class FS1PotionUseCoordinatorTest;
 friend class FS1QuickSlotCoordinatorTest;
 friend class FS1InventoryGridTest;
 friend class FS1InventoryMoveCoordinatorTest;
 friend class FS1InventoryStackCoordinatorTest;
 friend class FS1InventoryDiscardTest;
public:
 US1InventorySubsystem();
 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 virtual void Deinitialize() override;
 void BeginSession(uint64 Character,bool bSupported,bool bUseSupported=false,bool bMoveSupported=false,bool bStackSupported=false,bool bDiscardSupported=false);
 void ResetSession();
 void RequestPickup();
 void RequestUseItem(uint64 ItemId);
 void ToggleInventory();
 void CloseInventory();
 bool IsInventoryOpen() const {return InventoryWidget!=nullptr;}
 void RequestQuickSlot(int32 Slot);
 void RegisterQuickSlot(int32 Slot,uint64 ItemId);
 bool RegisterQuickSlotDrag(int32 Slot,uint64 ItemId,US1InventoryWidget* Source);
 bool RequestMoveItem(uint64 ItemId,uint32 SourceSlot,uint32 Destination,uint64 Version,US1InventoryWidget* Source);
 bool HandleInventoryDrop(uint64 ItemId,uint32 SourceSlot,uint32 Destination,uint64 Target,uint64 Version,US1InventoryWidget* Source,bool bSplit,bool bSwap);
 uint64 GetInventoryVersion() const {return State.GetVersion();}
 bool HandleDiscardDrop(uint64 Item,uint32 Source,uint64 Version,US1InventoryWidget* Owner);
 void ClearQuickSlot(int32 Slot);
 TSubclassOf<US1QuickSlotWidget> GetQuickSlotWidgetClass() const {return QuickSlotWidgetClass;}
 TSubclassOf<US1InventoryWidget> GetInventoryWidgetClass() const {return InventoryWidgetClass;}
private:
 bool Tick(float Delta);
 bool HasDiscardWork() const {return DiscardWidget!=nullptr||!DiscardRequest.empty();}
 bool ValidateDiscard(uint64 Item,uint32 Source,uint64 Version,uint32 Amount) const;
 void ConfirmDiscard(uint32 Amount);
 void CancelDiscard();
 void HandleDiscard(const Protocol::S_PLAYER_FIELD_DROP_RESULT& Packet);
 AS1MyPlayer* LocalPlayer() const;
 void QueryInventory();
 void HandlePickup(const Protocol::S_ITEM_PICKUP_RESULT& Packet);
 void HandleInventory(const Protocol::S_INVENTORY_SNAPSHOT& Packet);
 void HandleUse(const Protocol::S_ITEM_USE_RESULT& Packet);
 void HandleMove(const Protocol::S_INVENTORY_MOVE_RESULT& Packet);
 void HandleStack(const Protocol::S_INVENTORY_STACK_ACTION_RESULT& Packet);
 bool ValidateStack(uint32 Action,uint64 Item,uint32 Source,uint32 Destination,uint64 Target,uint64 Version,uint32 Amount) const;
 bool RequestStack(uint32 Action,uint64 Item,uint32 Source,uint32 Destination,uint64 Target,uint64 Version,uint32 Amount);
 void ConfirmSplit(uint32 Amount);
 void CancelSplit();
 void RefreshView();
 void RefreshQuickView();
 void LoadQuickSlots();
 void SaveQuickSlots();
 bool CanUseQuickSlot() const;
 FString SelectDrop() const;
 static std::string NewRequest();
 static bool DropBytes(const FString& Key,std::string& Bytes);
 UPROPERTY(Transient) TObjectPtr<US1InventoryWidget> InventoryWidget;
 UPROPERTY(Transient) TObjectPtr<US1QuickSlotWidget> QuickWidget;
 UPROPERTY(Transient) TObjectPtr<US1InventorySplitWidget> SplitWidget;
 UPROPERTY(Transient) TObjectPtr<US1InventoryDiscardWidget> DiscardWidget;
 UPROPERTY(Transient) TObjectPtr<US1InventoryDropZoneWidget> DropZoneWidget;
 UPROPERTY() TSubclassOf<US1InventoryDiscardWidget> DiscardWidgetClass;
 UPROPERTY() TSubclassOf<US1InventorySplitWidget> SplitWidgetClass;
 // Exact native default reference makes the editable Widget Blueprint discoverable to the cooker.
 UPROPERTY() TSubclassOf<US1QuickSlotWidget> QuickSlotWidgetClass;
 UPROPERTY() TSubclassOf<US1InventoryWidget> InventoryWidgetClass;
 FS1QuickSlotState QuickSlots;
 FString QuickIdentity;
 FTSTicker::FDelegateHandle TickHandle;
 FS1InventoryState State;
 uint64 CharacterId=0;
 bool bEnabled=false,bInitialQuery=false,bQueryPending=false,bPickupUnconfirmed=false;
 std::string QueryRequest,PickupRequest,PickupDrop;
 FString PendingKey,Status;
 double QueryAt=0,PickupAt=0;
 bool bUseEnabled=false,bUseUnconfirmed=false;
 std::string UseRequest;
 uint64 UseItem=0;
 double UseAt=0;
 bool bMoveEnabled=false,bMoveReconciling=false;
 std::string MoveRequest;
 uint64 MoveItem=0,MoveVersion=0;
 uint32 MoveSource=0,MoveDestination=0;
 double MoveAt=0;
 bool bStackEnabled=false,bStackReconciling=false;
 std::string StackRequest;
 uint64 StackItem=0,StackVersion=0;
 uint32 StackAction=0,StackSource=0,StackDestination=0;
 double StackAt=0;
 uint64 SplitItem=0,SplitVersion=0;
 uint32 SplitSource=0,SplitDestination=0;
 bool bDiscardEnabled=false,bDiscardReconciling=false;
 std::string DiscardRequest;
 uint64 DiscardItem=0,DiscardVersion=0;
 uint32 DiscardSource=0,DiscardAmount=0,DiscardOriginalCount=0;
 double DiscardAt=0;
};
