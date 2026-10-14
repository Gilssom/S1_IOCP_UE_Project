#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1InventorySubsystem.h"
#include "UI/S1InventoryWidget.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InventoryMoveCoordinatorTest,"S1.Inventory.MoveCoordinator",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InventoryMoveCoordinatorTest::RunTest(const FString&)
{
 auto* GI=NewObject<UGameInstance>();auto* C=NewObject<US1InventorySubsystem>(GI);
 auto* View=NewObject<US1InventoryWidget>(GI);
 C->BeginSession(42,true,true,true);C->InventoryWidget=View;
 Protocol::S_INVENTORY_SNAPSHOT Snapshot;Snapshot.set_character_id(42);Snapshot.set_success(true);Snapshot.set_inventory_version(3);
 auto* Entry=Snapshot.add_entries();Entry->set_item_instance_id(8);Entry->set_item_definition_id(20001);Entry->set_count(7);Entry->set_slot_index(0);
 TestTrue(TEXT("Initial snapshot"),C->State.Apply(Snapshot));
 TestFalse(TEXT("Foreign window rejected"),C->RequestMoveItem(8,0,1,3,nullptr));
 TestFalse(TEXT("Locked slot rejected"),C->RequestMoveItem(8,0,20,3,View));
 TestFalse(TEXT("Same slot rejected"),C->RequestMoveItem(8,0,0,3,View));
 TestFalse(TEXT("Stale drag rejected"),C->RequestMoveItem(8,0,1,2,View));
 TestFalse(TEXT("Wrong source rejected"),C->RequestMoveItem(8,1,2,3,View));
 C->MoveRequest=std::string(16,'m');C->MoveItem=8;C->MoveSource=0;C->MoveDestination=19;C->MoveVersion=3;
 TestFalse(TEXT("Pending movement blocks quick-slot registration"),C->RegisterQuickSlotDrag(0,8,View));
 Protocol::S_INVENTORY_MOVE_RESULT Response;Response.set_request_id(std::string(16,'x'));Response.set_character_id(42);
 Response.set_item_instance_id(8);Response.set_source_slot(0);Response.set_destination_slot(19);Response.set_decision(1);Response.set_inventory_version(4);
 C->HandleMove(Response);TestFalse(TEXT("Old response ignored"),C->bMoveReconciling);
 Response.set_request_id(C->MoveRequest);Response.set_character_id(99);C->HandleMove(Response);TestFalse(TEXT("Foreign character ignored"),C->bMoveReconciling);
 Response.set_character_id(42);C->HandleMove(Response);
 TestTrue(TEXT("Committed response requests reconciliation"),C->bMoveReconciling);
 TestEqual(TEXT("No optimistic relocation"),C->State.GetEntries()[0].slot_index(),uint32(0));
 TestFalse(TEXT("Older snapshot cannot unlock"),C->State.ReadyForUse());
 C->bQueryPending=true;C->QueryRequest=std::string(16,'q');Snapshot.set_request_id(C->QueryRequest);
 C->HandleInventory(Snapshot);TestTrue(TEXT("Pre-commit snapshot leaves move pending"),!C->MoveRequest.empty());
 Snapshot.set_inventory_version(4);Snapshot.mutable_entries(0)->set_slot_index(19);C->bQueryPending=true;
 C->HandleInventory(Snapshot);TestTrue(TEXT("Confirmed snapshot unlocks"),C->MoveRequest.empty());
 TestEqual(TEXT("Server slot applied"),C->State.GetEntries()[0].slot_index(),uint32(19));
 TestEqual(TEXT("Count unchanged"),C->State.GetCount(),int64(7));
 C->MoveRequest=std::string(16,'n');C->MoveItem=8;C->MoveVersion=4;C->MoveSource=19;C->MoveDestination=2;
 Response.set_request_id(std::string(16,'m'));C->HandleMove(Response);
 TestTrue(TEXT("Past acceptance cannot clear newer move"),C->MoveRequest==std::string(16,'n'));
 Response.set_request_id(C->MoveRequest);Response.set_source_slot(19);Response.set_destination_slot(2);Response.set_decision(3);Response.set_reason(6);
 C->HandleMove(Response);TestTrue(TEXT("Unconfirmed is read-only reconciliation"),C->bMoveReconciling);
 TestTrue(TEXT("Unknown keeps original request, never resends"),!C->MoveRequest.empty());
 C->InventoryWidget=nullptr;C->ResetSession();TestTrue(TEXT("Session reset clears requests"),C->MoveRequest.empty());
 TestFalse(TEXT("Session reset clears capability"),C->bMoveEnabled);
 C->BeginSession(42,true,true);C->InventoryWidget=View;
 TestFalse(TEXT("Legacy server disables move"),C->RequestMoveItem(8,0,1,0,View));C->InventoryWidget=nullptr;C->ResetSession();
 return !HasAnyErrors();
}
#endif
