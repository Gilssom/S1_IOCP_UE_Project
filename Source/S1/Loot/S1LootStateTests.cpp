#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1LootState.h"
#include "World/S1WorldEntitySubsystem.h"
#include "Loot/S1LootActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Protocol.pb.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1LootStateTest,"S1.Loot.VisualState",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1LootStateTest::RunTest(const FString& Parameters)
{
 FS1LootState State;
 Protocol::S_LOOT_SNAPSHOT Snapshot;Snapshot.set_contract_version(1);
 auto* Drop=Snapshot.add_drops();Drop->set_drop_id(std::string(16,'\x21'));
 Drop->set_owner_character_id(42);Drop->set_item_definition_id(20001);Drop->set_count(1);
 Drop->set_z(-0.5f);Drop->set_expires_utc_ms(123);Drop->set_definition_version(1);
 TestFalse(TEXT("No display without negotiation"),State.ApplySnapshot(Snapshot));
 State.Reset(true);TestTrue(TEXT("Initial snapshot sequence0"),State.ApplySnapshot(Snapshot));
 TestEqual(TEXT("Loading data retained, no local clock expiry"),State.GetDrops().Num(),1);
 Protocol::S_LOOT_CREATED Created;Created.mutable_drop()->CopyFrom(*Drop);Created.set_event_sequence(1);
 TestTrue(TEXT("Duplicate ID does not duplicate data"),State.ApplyCreated(Created));
 TestFalse(TEXT("Duplicate event ignored"),State.ApplyCreated(Created));
 TestEqual(TEXT("One drop"),State.GetDrops().Num(),1);
 auto Bad=Created;Bad.set_event_sequence(2);Bad.mutable_drop()->set_x(std::numeric_limits<float>::quiet_NaN());
 TestFalse(TEXT("Nonfinite positions rejected"),State.ApplyCreated(Bad));
 TestEqual(TEXT("Invalid event does not consume sequence"),State.GetSequence(),uint64(1));
 Bad=Created;Bad.set_event_sequence(2);Bad.mutable_drop()->set_owner_character_id(43);
 TestFalse(TEXT("Immutable drop owner"),State.ApplyCreated(Bad));
 Protocol::S_LOOT_REMOVED Removed;Removed.set_drop_id(Drop->drop_id());
 Removed.set_reason(Protocol::LOOT_REMOVAL_EXPIRED);Removed.set_event_sequence(2);
 TestTrue(TEXT("Authoritative expiry retires"),State.ApplyRemoved(Removed));
 TestEqual(TEXT("No drop after expiry"),State.GetDrops().Num(),0);
 TestFalse(TEXT("Late create cannot resurrect"),State.ApplyCreated(Created));
 TestFalse(TEXT("Old snapshot cannot resurrect"),State.ApplySnapshot(Snapshot));
 Snapshot.set_snapshot_sequence(3);TestTrue(TEXT("New reconnect snapshot"),State.ApplySnapshot(Snapshot));
 auto InvalidSnapshot=Snapshot;InvalidSnapshot.set_snapshot_sequence(4);
 InvalidSnapshot.add_drops()->CopyFrom(*Drop);
 TestFalse(TEXT("Duplicate snapshot identities rejected atomically"),State.ApplySnapshot(InvalidSnapshot));
 TestEqual(TEXT("Previous valid state retained"),State.GetDrops().Num(),1);
 State.ResetForRegion();
 TestTrue(TEXT("Region change preserves negotiated loot presentation"),State.IsEnabled());
 TestEqual(TEXT("Region change clears old region drops"),State.GetDrops().Num(),0);
 TestEqual(TEXT("Region change clears old sequence"),State.GetSequence(),uint64(0));
 TestFalse(TEXT("Created event waits for destination snapshot"),State.ApplyCreated(Created));
 Snapshot.set_snapshot_sequence(0);
 TestTrue(TEXT("Destination snapshot accepted with fresh sequence"),State.ApplySnapshot(Snapshot));
 TestTrue(TEXT("Destination create event accepted"),State.ApplyCreated(Created));
 State.Reset();TestEqual(TEXT("Session cleanup"),State.GetDrops().Num(),0);
 TestFalse(TEXT("Old packet after disconnect ignored"),State.ApplySnapshot(Snapshot));
 State.ResetForRegion();TestFalse(TEXT("Region reset does not enable unnegotiated presentation"),State.IsEnabled());
 FS1LootPresentation Public;
 auto PublicPacket=*Drop;PublicPacket.set_source_kind(2);PublicPacket.set_count(99);
 TestTrue(TEXT("Public whole stack supported"),FS1LootState::Decode(PublicPacket,Public));
 TestEqual(TEXT("Public count retained"),Public.Count,99u);
 TestEqual(TEXT("Public provenance retained"),Public.SourceKind,2u);
 PublicPacket.set_source_kind(1);TestFalse(TEXT("Monster drop remains single potion"),FS1LootState::Decode(PublicPacket,Public));
 PublicPacket.set_source_kind(3);TestFalse(TEXT("Unknown source rejected"),FS1LootState::Decode(PublicPacket,Public));
 PublicPacket.set_source_kind(2);PublicPacket.set_count(100);TestFalse(TEXT("Over stack rejected"),FS1LootState::Decode(PublicPacket,Public));
 PublicPacket.set_count(0);TestFalse(TEXT("Zero rejected"),FS1LootState::Decode(PublicPacket,Public));
 Protocol::C_ENTER_GAME Legacy;Protocol::S_ENTER_GAME LegacyServer;
 TestEqual(TEXT("Legacy field opt-in absent"),Legacy.player_field_drop_contract_version(),0u);
 TestEqual(TEXT("Legacy field capability absent"),LegacyServer.player_field_drop_contract_version(),0u);

 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("World"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 // CreateWorld initializes the world already; initializing it twice duplicates WorldSettings.
 auto* Registry=World->GetSubsystem<US1WorldEntitySubsystem>();
 if(TestNotNull(TEXT("World registry"),Registry))
 {
  State.Reset(true);State.ApplySnapshot(Snapshot);
  TestTrue(TEXT("World consumes server state"),Registry->SyncLootPresentation(State.GetDrops()));
  TestTrue(TEXT("Repeated flush is idempotent"),Registry->SyncLootPresentation(State.GetDrops()));
  int32 Count=0;for(TActorIterator<AS1LootActor> It(World);It;++It)++Count;
  TestEqual(TEXT("One world actor"),Count,1);
  TestTrue(TEXT("Empty authoritative state removes"),Registry->SyncLootPresentation({}));
  Count=0;for(TActorIterator<AS1LootActor> It(World);It;++It)++Count;
  TestEqual(TEXT("No world drop after removal"),Count,0);
  Registry->ResetLootRegistry();
 }
 World->DestroyWorld(false);GEngine->DestroyWorldContext(World);
 return !HasAnyErrors(); // No GPU or real socket acceptance claimed.
}
#endif
