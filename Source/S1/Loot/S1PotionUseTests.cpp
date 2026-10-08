#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1InventorySubsystem.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "World/S1WorldEntitySubsystem.h"
#include "Game/S1Player.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TextRenderComponent.h"
#include "NiagaraComponent.h"
#include "ClientPacketHandler.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PotionUseTransportTest,"S1.Loot.UseTransport",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1PotionUseTransportTest::RunTest(const FString& Parameters)
{
 Protocol::C_ENTER_GAME Legacy;TestEqual(TEXT("Absent use opt-in is legacy"),Legacy.item_use_contract_version(),0u);
 Protocol::S_ENTER_GAME Server;TestEqual(TEXT("Absent server capability disables use"),Server.item_use_contract_version(),0u);
 TestEqual(TEXT("Existing inventory ID unchanged"),int32(PKT_S_INVENTORY_SNAPSHOT),1029);
 TestEqual(TEXT("Appended use request"),int32(PKT_C_ITEM_USE),1030);
 TestEqual(TEXT("Appended private result"),int32(PKT_S_ITEM_USE_RESULT),1031);
 TestEqual(TEXT("Appended public heal"),int32(PKT_S_PLAYER_HEALED),1032);
 Protocol::C_ITEM_USE Use;Use.set_request_id(std::string(16,'r'));Use.set_item_instance_id(8);Use.set_item_definition_id(20001);Use.set_contract_version(1);
 Protocol::C_ITEM_USE Parsed;TestTrue(TEXT("Use roundtrip"),Parsed.ParseFromString(Use.SerializeAsString()));
 TestEqual(TEXT("Opaque request identity"),int32(Parsed.request_id().size()),16);
 TestEqual(TEXT("Chosen instance"),Parsed.item_instance_id(),uint64(8));
 Protocol::S_PLAYER_HEALED Heal;Heal.set_object_id(42);Heal.set_effect_event_id(std::string(16,'e'));Heal.set_actual_heal(10);
 Heal.set_current_hp(100);Heal.set_max_hp(100);Heal.set_room_event_sequence(9);
 Protocol::S_PLAYER_HEALED Copy;TestTrue(TEXT("Public heal roundtrip"),Copy.ParseFromString(Heal.SerializeAsString()));
 TestEqual(TEXT("Public event keeps actual heal"),Copy.actual_heal(),10u);
 TestEqual(TEXT("Exact selected default FX"),GetDefault<US1PotionPresentationSettings>()->HealingEffect.ToString(),
  FString(TEXT("/Game/VFX_StatusEffect01/FXS/FXS_healing000.FXS_healing000")));
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PotionUseCoordinatorTest,"S1.Loot.UseCoordinator",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1PotionUseCoordinatorTest::RunTest(const FString& Parameters)
{
 auto* Instance=NewObject<UGameInstance>();auto* Coordinator=NewObject<US1InventorySubsystem>(Instance);
 Coordinator->BeginSession(42,true,true);
 Protocol::S_INVENTORY_SNAPSHOT Snapshot;Snapshot.set_character_id(42);Snapshot.set_success(true);Snapshot.set_inventory_version(1);
 auto* Entry=Snapshot.add_entries();Entry->set_item_instance_id(8);Entry->set_item_definition_id(20001);Entry->set_count(1);Entry->set_slot_index(0);
 TestTrue(TEXT("Server stack snapshot"),Coordinator->State.Apply(Snapshot));
 auto Conflict=Snapshot;Conflict.mutable_entries(0)->set_item_instance_id(9);
 TestFalse(TEXT("Same-version different instance cannot silently replace selection"),Coordinator->State.Apply(Conflict));
 Coordinator->UseRequest=std::string(16,'u');Coordinator->UseItem=8;
 Protocol::S_ITEM_USE_RESULT Result;Result.set_request_id(std::string(16,'x'));Result.set_character_id(42);
 Result.set_item_instance_id(8);Result.set_item_definition_id(20001);Result.set_decision(Protocol::ITEM_USE_UNCONFIRMED);
 Result.set_reason(Protocol::ITEM_USE_REASON_DATABASE_UNAVAILABLE);Coordinator->HandleUse(Result);
 TestFalse(TEXT("Foreign request ignored"),Coordinator->bUseUnconfirmed);
 Result.set_request_id(Coordinator->UseRequest);Coordinator->HandleUse(Result);
 TestTrue(TEXT("Unknown retains ID for explicit retry"),Coordinator->bUseUnconfirmed);
 TestEqual(TEXT("Unknown never decrements"),Coordinator->State.GetCount(),int64(1));
 Result.set_decision(Protocol::ITEM_USE_APPLIED);Result.set_reason(Protocol::ITEM_USE_REASON_NONE);
 Result.set_operation_id(std::string(16,'o'));Result.set_inventory_version(2);Result.set_actual_heal(50);Result.set_resulting_count(0);
 Coordinator->HandleUse(Result);
 TestTrue(TEXT("Successful use clears pending input"),Coordinator->UseRequest.empty());
 TestFalse(TEXT("Known newer version disables stale stack use"),Coordinator->State.ReadyForUse());
 TestEqual(TEXT("Result does not invent quantity"),Coordinator->State.GetCount(),int64(1));
 TestFalse(TEXT("Old snapshot ignored after use"),Coordinator->State.Apply(Snapshot));
 Snapshot.set_inventory_version(2);Snapshot.clear_entries();TestTrue(TEXT("Last item disappears only on new snapshot"),Coordinator->State.Apply(Snapshot));
 TestEqual(TEXT("Server zero count"),Coordinator->State.GetCount(),int64(0));
 Coordinator->UseRequest=std::string(16,'n');Coordinator->UseItem=10;Coordinator->HandleUse(Result);
 TestTrue(TEXT("Old success cannot clear newer request"),Coordinator->UseRequest==std::string(16,'n'));
 Coordinator->ResetSession();Coordinator->HandleUse(Result);
 TestFalse(TEXT("Disconnect removes use capability"),Coordinator->bUseEnabled);
 TestTrue(TEXT("Disconnect clears pending use"),Coordinator->UseRequest.empty());
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PotionHealingTest,"S1.Loot.HealingPresentation",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1PotionHealingTest::RunTest(const FString& Parameters)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("Transient world"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Entities=World->GetSubsystem<US1WorldEntitySubsystem>();auto* Player=World->SpawnActor<AS1Player>();
 if(TestNotNull(TEXT("Registry"),Entities)&&TestNotNull(TEXT("Player"),Player))
 {
  Entities->Players.Add(42,Player);Player->SetActorLocation(FVector(100,200,300));
  const FVector Location=Player->GetActorLocation();const auto Mode=Player->GetCharacterMovement()->MovementMode;
  Protocol::S_PLAYER_HEALED Heal;Heal.set_object_id(42);Heal.set_effect_event_id(std::string(16,'e'));
  Heal.set_actual_heal(50);Heal.set_current_hp(90);Heal.set_max_hp(100);Heal.set_room_event_sequence(2);
  TestTrue(TEXT("Confirmed heal applies"),Entities->ApplyPlayerHealed(Heal));
  TestEqual(TEXT("Heal does not teleport"),Player->GetActorLocation(),Location);
  TestEqual(TEXT("Heal does not reset movement mode"),Player->GetCharacterMovement()->MovementMode,Mode);
  auto* Label=Player->FindComponentByClass<UTextRenderComponent>();
  if(TestNotNull(TEXT("HP label"),Label))TestEqual(TEXT("Authoritative HP label"),Label->Text.ToString(),FString(TEXT("Knight 90/100")));
  TestFalse(TEXT("Duplicate effect cannot reapply"),Entities->ApplyPlayerHealed(Heal));
  Protocol::S_DAMAGE_RESULT Damage;Damage.set_source_object_id(9);Damage.set_target_object_id(42);Damage.set_action_instance_id(1);
  Damage.set_damage(10);Damage.set_current_hp(40);Damage.set_max_hp(100);Damage.set_room_event_sequence(1);
  TestFalse(TEXT("Old damage cannot undo new healing"),Entities->ApplyDamageResult(Damage));
  Damage.set_room_event_sequence(3);TestTrue(TEXT("New damage applies"),Entities->ApplyDamageResult(Damage));
  Protocol::S_PLAYER_STATUS Death;Death.set_object_id(42);Death.set_current_hp(0);Death.set_max_hp(100);Death.set_is_dead(true);Death.set_room_event_sequence(4);
  TestTrue(TEXT("Authoritative death"),Entities->ApplyPlayerStatus(Death));
  TestFalse(TEXT("Death cleans owned FX"),Entities->HealingEffects.Contains(42));
  Damage.set_action_instance_id(2);Damage.set_room_event_sequence(5);
  TestFalse(TEXT("Damage HP update cannot revive dead actor"),Entities->ApplyDamageResult(Damage));
  Heal.set_effect_event_id(std::string(16,'f'));Heal.set_room_event_sequence(5);
  TestFalse(TEXT("Heal cannot resurrect dead player"),Entities->ApplyPlayerHealed(Heal));
  Death.set_is_dead(false);Death.set_current_hp(90);Death.set_room_event_sequence(6);
  TestTrue(TEXT("Only fresh authoritative alive status releases death"),Entities->ApplyPlayerStatus(Death));
  Heal.set_actual_heal(10);Heal.set_current_hp(100);Heal.set_room_event_sequence(7);
  TestTrue(TEXT("New life can heal normally"),Entities->ApplyPlayerHealed(Heal));
  Entities->ResetRegistry();TestTrue(TEXT("Reset clears effect dedup"),Entities->RecentHealIds.IsEmpty());
  TestFalse(TEXT("Old actor cannot receive heal after reset"),Entities->ApplyPlayerHealed(Heal));Player->Destroy();
 }
 World->DestroyWorld(false);GEngine->DestroyWorldContext(World);
 return !HasAnyErrors(); // No real socket/rendered GPU or user acceptance claim.
}
#endif
