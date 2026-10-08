#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "Loot/S1InventoryState.h"
#include "Loot/S1QuickSlotState.h"
#include "S1InventorySubsystem.generated.h"

class US1InventoryWidget;
class US1QuickSlotWidget;
class AS1MyPlayer;

/** Request coordination and inventory UI state, not world actor ownership or transport. */
UCLASS()
class S1_API US1InventorySubsystem : public UGameInstanceSubsystem
{
 GENERATED_BODY()
 friend class FS1PickupCoordinatorTest;
 friend class FS1PotionUseCoordinatorTest;
 friend class FS1QuickSlotCoordinatorTest;
public:
 US1InventorySubsystem();
 virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 virtual void Deinitialize() override;
 void BeginSession(uint64 Character,bool bSupported,bool bUseSupported=false);
 void ResetSession();
 void RequestPickup();
 void RequestUseItem(uint64 ItemId);
 void ToggleInventory();
 void CloseInventory();
 bool IsInventoryOpen() const {return InventoryWidget!=nullptr;}
 void RequestQuickSlot(int32 Slot);
 void RegisterQuickSlot(int32 Slot,uint64 ItemId);
 void ClearQuickSlot(int32 Slot);
 TSubclassOf<US1QuickSlotWidget> GetQuickSlotWidgetClass() const {return QuickSlotWidgetClass;}
 TSubclassOf<US1InventoryWidget> GetInventoryWidgetClass() const {return InventoryWidgetClass;}
private:
 bool Tick(float Delta);
 AS1MyPlayer* LocalPlayer() const;
 void QueryInventory();
 void HandlePickup(const Protocol::S_ITEM_PICKUP_RESULT& Packet);
 void HandleInventory(const Protocol::S_INVENTORY_SNAPSHOT& Packet);
 void HandleUse(const Protocol::S_ITEM_USE_RESULT& Packet);
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
};
