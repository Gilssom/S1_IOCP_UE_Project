#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1InventoryState.h"
#include "Game/S1MyPlayer.h"
#include "Components/CapsuleComponent.h"
#include "Loot/S1InventorySubsystem.h"
#include "Engine/GameInstance.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InventoryStateTest,"S1.Loot.InventoryState",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InventoryStateTest::RunTest(const FString& Parameters)
{
 FS1InventoryState State;Protocol::S_INVENTORY_SNAPSHOT Packet;
 Packet.set_character_id(42);Packet.set_success(true);Packet.set_inventory_version(1);
 auto* Entry=Packet.add_entries();Entry->set_item_instance_id(2);Entry->set_item_definition_id(20001);
 Entry->set_count(1);Entry->set_slot_index(0);
 TestFalse(TEXT("No session cannot apply"),State.Apply(Packet));State.Reset(42);
 TestTrue(TEXT("Confirmed server snapshot"),State.Apply(Packet));
 TestEqual(TEXT("Server count only"),State.GetCount(),int64(1));
 auto Bad=Packet;Bad.set_inventory_version(2);Bad.add_entries()->CopyFrom(*Entry);
 TestFalse(TEXT("Duplicate identity/slot rejected atomically"),State.Apply(Bad));
 TestEqual(TEXT("Rejected snapshot retained old count"),State.GetCount(),int64(1));
 Bad=Packet;Bad.set_character_id(43);TestFalse(TEXT("Foreign character rejected"),State.Apply(Bad));
 Bad=Packet;Bad.mutable_entries(0)->set_count(2);
 TestFalse(TEXT("Equal-version conflicting count"),State.Apply(Bad));
 State.RequireVersion(3);TestFalse(TEXT("Precommit query cannot roll back known commit"),State.Apply(Packet));
 TestEqual(TEXT("Commit result does not optimistically add"),State.GetCount(),int64(1));
 Packet.set_inventory_version(3);Packet.mutable_entries(0)->set_count(2);
 TestTrue(TEXT("Postcommit server snapshot"),State.Apply(Packet));
 TestEqual(TEXT("Confirmed count two"),State.GetCount(),int64(2));
 Bad=Packet;Bad.mutable_entries(0)->set_count(100);TestFalse(TEXT("Over-stack rejected"),State.Apply(Bad));
 Bad=Packet;Bad.mutable_entries(0)->set_slot_index(20);TestFalse(TEXT("Over-capacity slot rejected"),State.Apply(Bad));
 Packet.set_inventory_version(2);TestFalse(TEXT("Old snapshot ignored"),State.Apply(Packet));
 State.Reset();TestEqual(TEXT("Disconnect clears count"),State.GetCount(),int64(0));
 TestFalse(TEXT("Old session packet ignored"),State.Apply(Packet));
 UClass* PlayerClass=LoadClass<AS1MyPlayer>(nullptr,TEXT("/Game/Blueprints/BP_MyPlayer.BP_MyPlayer_C"));
 if(!TestNotNull(TEXT("Existing player BP loads"),PlayerClass))return false;
 TestEqual(TEXT("Server foot conversion matches actual player capsule"),
  PlayerClass->GetDefaultObject<AS1MyPlayer>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),96.f);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PickupCoordinatorTest,"S1.Loot.PickupCoordinator",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1PickupCoordinatorTest::RunTest(const FString& Parameters)
{
 auto* Instance=NewObject<UGameInstance>();
 auto* Coordinator=NewObject<US1InventorySubsystem>(Instance);
 Coordinator->BeginSession(42,true);
 Coordinator->PickupRequest=std::string(16,'r');Coordinator->PickupDrop=std::string(16,'d');Coordinator->PendingKey=TEXT("test");
 Protocol::S_ITEM_PICKUP_RESULT Result;Result.set_request_id(std::string(16,'x'));Result.set_drop_id(Coordinator->PickupDrop);
 Result.set_decision(Protocol::ITEM_PICKUP_REJECTED);Result.set_reason(Protocol::ITEM_REASON_DEAD);
 Coordinator->HandlePickup(Result);
 TestFalse(TEXT("Foreign response cannot clear active request"),Coordinator->PickupRequest.empty());
 Result.set_request_id(Coordinator->PickupRequest);Result.set_decision(Protocol::ITEM_PICKUP_UNCONFIRMED);
 Result.set_reason(Protocol::ITEM_REASON_DATABASE_UNAVAILABLE);Coordinator->HandlePickup(Result);
 TestTrue(TEXT("Unconfirmed retains same request for manual retry"),Coordinator->bPickupUnconfirmed);
 TestTrue(TEXT("Same request retained"),Coordinator->PickupRequest==std::string(16,'r'));
 Result.set_decision(Protocol::ITEM_PICKUP_COMMITTED);Result.set_reason(Protocol::ITEM_REASON_NONE);
 Result.set_inventory_version(2);Result.set_item_instance_id(8);Result.set_resulting_count(1);
 Coordinator->HandlePickup(Result);
 TestTrue(TEXT("Commit clears request"),Coordinator->PickupRequest.empty());
 TestEqual(TEXT("Commit does not invent optimistic quantity"),Coordinator->State.GetCount(),int64(0));
 Coordinator->PickupRequest=std::string(16,'n');Coordinator->PickupDrop=std::string(16,'e');
 Coordinator->HandlePickup(Result);
 TestTrue(TEXT("Old successful response cannot clear newer request"),Coordinator->PickupRequest==std::string(16,'n'));
 Coordinator->bQueryPending=true;Coordinator->QueryRequest=std::string(16,'q');
 Protocol::S_INVENTORY_SNAPSHOT Snapshot;Snapshot.set_request_id(Coordinator->QueryRequest);
 Snapshot.set_character_id(42);Snapshot.set_success(true);Snapshot.set_inventory_version(1);
 Coordinator->HandleInventory(Snapshot);
 TestFalse(TEXT("Precommit stale inventory rejected"),Coordinator->State.HasSnapshot());
 Coordinator->bQueryPending=true;Snapshot.set_inventory_version(2);
 auto* Entry=Snapshot.add_entries();Entry->set_item_instance_id(8);Entry->set_item_definition_id(20001);Entry->set_count(1);
 Coordinator->HandleInventory(Snapshot);
 TestEqual(TEXT("Only snapshot updates quantity"),Coordinator->State.GetCount(),int64(1));
 Coordinator->ResetSession();Coordinator->HandlePickup(Result);
 TestEqual(TEXT("Disconnect resets inventory and stale responses cannot restore"),Coordinator->State.GetCount(),int64(0));
 TestTrue(TEXT("Disconnect clears pending request"),Coordinator->PickupRequest.empty());
 return true;
}
#endif
