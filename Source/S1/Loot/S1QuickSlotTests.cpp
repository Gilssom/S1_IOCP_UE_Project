#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1QuickSlotState.h"
#include "Loot/S1QuickSlotSaveGame.h"
#include "Loot/S1InventorySubsystem.h"
#include "UI/S1InventoryWidget.h"
#include "Game/S1MyPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "InputAction.h"
#include "Loot/S1QuickSlotInputGate.h"
#include "Widgets/SViewport.h"
#include "Widgets/Input/SEditableText.h"

// UE's SPIEViewport is private to UnrealEd; exercise the same derived-type mismatch without an editor dependency.
class SQuickSlotDerivedViewport : public SViewport
{
public:
 SLATE_BEGIN_ARGS(SQuickSlotDerivedViewport){} SLATE_END_ARGS()
 void Construct(const FArguments& Args){SViewport::Construct(SViewport::FArguments());}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuickSlotFocusTest,"S1.QuickSlots.OwningViewportFocus",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuickSlotFocusTest::RunTest(const FString& Parameters)
{
 TSharedPtr<SWidget> PIE=SNew(SQuickSlotDerivedViewport);
 TSharedPtr<SWidget> Packaged=SNew(SViewport);
 TSharedPtr<SWidget> OtherPIE=SNew(SQuickSlotDerivedViewport);
 TSharedPtr<SWidget> TextInput=SNew(SEditableText);
 TestTrue(TEXT("Reproduces old exact-type rejection"),PIE->GetType()!=FName(TEXT("SViewport")));
 TestTrue(TEXT("Owning derived PIE viewport permitted"),FS1QuickSlotInputGate::AllowsFocus(PIE,PIE,false));
 TestTrue(TEXT("Owning packaged viewport permitted"),FS1QuickSlotInputGate::AllowsFocus(Packaged,Packaged,false));
 TestFalse(TEXT("Different PIE instance blocked"),FS1QuickSlotInputGate::AllowsFocus(OtherPIE,PIE,false));
 TestFalse(TEXT("Focused text input blocked"),FS1QuickSlotInputGate::AllowsFocus(TextInput,PIE,false));
 TestFalse(TEXT("Modal blocked even with viewport focus"),FS1QuickSlotInputGate::AllowsFocus(PIE,PIE,true));
 TestFalse(TEXT("No keyboard focus blocked"),FS1QuickSlotInputGate::AllowsFocus(nullptr,PIE,false));
 TestFalse(TEXT("No owning viewport blocked"),FS1QuickSlotInputGate::AllowsFocus(PIE,nullptr,false));
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuickSlotStateTest,"S1.QuickSlots.StateAndSave",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuickSlotStateTest::RunTest(const FString& Parameters)
{
 FS1QuickSlotState Slots;FS1InventoryState Inventory;Inventory.Reset(42);
 for(int32 I=0;I<4;++I)TestTrue(TEXT("Duplicate definition registration"),Slots.Set(I,20001));
 TestFalse(TEXT("Unsupported item"),Slots.Set(0,11002));TestFalse(TEXT("Invalid slot"),Slots.Set(4,20001));
 Protocol::S_INVENTORY_SNAPSHOT Snapshot;Snapshot.set_character_id(42);Snapshot.set_success(true);Snapshot.set_inventory_version(1);
 auto* Entry=Snapshot.add_entries();Entry->set_item_instance_id(99);Entry->set_slot_index(7);Entry->set_item_definition_id(20001);Entry->set_count(3);
 Entry=Snapshot.add_entries();Entry->set_item_instance_id(12);Entry->set_slot_index(1);Entry->set_item_definition_id(20001);Entry->set_count(2);
 TestTrue(TEXT("Validated multi-stack"),Inventory.Apply(Snapshot));
 for(int32 I=0;I<4;++I)TestEqual(TEXT("Fresh stable earliest stack for every slot"),Slots.Resolve(I,Inventory),uint64(12));
 TestEqual(TEXT("Shared total"),Inventory.GetCount(),int64(5));
 Inventory.RequireVersion(2);TestEqual(TEXT("Stale snapshot not usable"),Slots.Resolve(0,Inventory),uint64(0));
 Snapshot.set_inventory_version(2);Snapshot.mutable_entries()->DeleteSubrange(1,1);Inventory.Apply(Snapshot);
 TestEqual(TEXT("Consumed stack replaced at use time"),Slots.Resolve(0,Inventory),uint64(99));
 Snapshot.set_inventory_version(3);Snapshot.clear_entries();Inventory.Apply(Snapshot);
 TestEqual(TEXT("No stock"),Slots.Resolve(0,Inventory),uint64(0));TestEqual(TEXT("Zero does not clear registration"),Slots.Definitions[0],20001);
 Snapshot.set_inventory_version(4);Entry=Snapshot.add_entries();Entry->set_item_instance_id(123);Entry->set_slot_index(0);Entry->set_item_definition_id(20001);Entry->set_count(1);Inventory.Apply(Snapshot);
 TestEqual(TEXT("Reacquisition auto reactivates with new instance"),Slots.Resolve(0,Inventory),uint64(123));
 const FString Hash=FS1QuickSlotState::Namespace(TEXT("127.0.0.1:7777"),10,42);
 TestNotEqual(TEXT("Account isolation"),Hash,FS1QuickSlotState::Namespace(TEXT("127.0.0.1:7777"),11,42));
 TestNotEqual(TEXT("Character isolation"),Hash,FS1QuickSlotState::Namespace(TEXT("127.0.0.1:7777"),10,43));
 TestNotEqual(TEXT("Endpoint isolation"),Hash,FS1QuickSlotState::Namespace(TEXT("127.0.0.1:7778"),10,42));
 TestTrue(TEXT("Unauthenticated namespace rejected"),FS1QuickSlotState::Namespace(TEXT("127.0.0.1:7777"),0,42).IsEmpty());
 const FString TestSaveName=TEXT("S1QuickSlotTest_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
 auto* Saved=NewObject<US1QuickSlotSaveGame>();Saved->IdentityHash=Hash;Saved->Definitions=Slots.Definitions;
 TestTrue(TEXT("Actual local save write"),UGameplayStatics::SaveGameToSlot(Saved,TestSaveName,0));
 auto* Read=Cast<US1QuickSlotSaveGame>(UGameplayStatics::LoadGameFromSlot(TestSaveName,0));
 if(TestNotNull(TEXT("Actual reload"),Read))
 {
  FS1QuickSlotState Loaded;TestTrue(TEXT("Valid restore"),Loaded.Restore(Read->Version,Read->IdentityHash,Hash,Read->Definitions));
  TestEqual(TEXT("Four saved assignments"),Loaded.Definitions.Num(),4);TestEqual(TEXT("Slot4 survived reload"),Loaded.Definitions[3],20001);
  TestFalse(TEXT("Foreign session save rejected"),Loaded.Restore(1,Hash,TEXT("other"),Read->Definitions));
  TestEqual(TEXT("Foreign save clears"),Loaded.Definitions[0],0);
  TestFalse(TEXT("Corrupt size"),Loaded.Restore(1,Hash,Hash,{20001}));
  TestFalse(TEXT("Future schema"),Loaded.Restore(2,Hash,Hash,Read->Definitions));
  TestTrue(TEXT("Unsupported definition sanitized"),Loaded.Restore(1,Hash,Hash,{20001,999,0,20001}));TestEqual(TEXT("Invalid definition empty"),Loaded.Definitions[1],0);
  TestTrue(TEXT("Clear"),Loaded.Set(3,0));TestEqual(TEXT("Cleared definition"),Loaded.Definitions[3],0);
 }
 TestTrue(TEXT("Remove only this unique test save"),UGameplayStatics::DeleteGameInSlot(TestSaveName,0));
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuickSlotCoordinatorTest,"S1.QuickSlots.Coordinator",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuickSlotCoordinatorTest::RunTest(const FString& Parameters)
{
 auto* Instance=NewObject<UGameInstance>();auto* Coordinator=NewObject<US1InventorySubsystem>(Instance);
 Coordinator->BeginSession(42,true,true);
 Coordinator->InventoryWidget=NewObject<US1InventoryWidget>();
 Protocol::S_INVENTORY_SNAPSHOT Snapshot;Snapshot.set_character_id(42);Snapshot.set_success(true);Snapshot.set_inventory_version(1);
 auto* Entry=Snapshot.add_entries();Entry->set_item_instance_id(8);Entry->set_item_definition_id(20001);Entry->set_slot_index(0);Entry->set_count(1);
 Coordinator->State.Apply(Snapshot);
 for(int32 I=0;I<4;++I){Coordinator->RegisterQuickSlot(I,8);TestEqual(TEXT("Explicit registration uses selected owned definition"),Coordinator->QuickSlots.Definitions[I],20001);}
 Coordinator->ClearQuickSlot(2);TestEqual(TEXT("Explicit clear"),Coordinator->QuickSlots.Definitions[2],0);
 Coordinator->RegisterQuickSlot(2,999);TestEqual(TEXT("Foreign instance registration rejected"),Coordinator->QuickSlots.Definitions[2],0);
 Coordinator->InventoryWidget=nullptr;
 Coordinator->UseRequest=std::string(16,'u');Coordinator->UseItem=8;Coordinator->bUseUnconfirmed=true;
 Coordinator->RequestQuickSlot(0);Coordinator->RequestQuickSlot(1);
 TestTrue(TEXT("Unknown retains same ID across slots"),Coordinator->UseRequest==std::string(16,'u'));
 TestEqual(TEXT("Original unknown item retained"),Coordinator->UseItem,uint64(8));
 TestTrue(TEXT("No hotkey retry clears unknown"),Coordinator->bUseUnconfirmed);
 Protocol::S_ITEM_USE_RESULT Old;Old.set_character_id(41);Old.set_request_id(std::string(16,'u'));Old.set_item_instance_id(8);
 Old.set_item_definition_id(20001);Old.set_decision(Protocol::ITEM_USE_REJECTED);Old.set_reason(Protocol::ITEM_USE_REASON_FULL_HP);
 Coordinator->HandleUse(Old);TestTrue(TEXT("Other character result ignored"),!Coordinator->UseRequest.empty());
 Old.set_character_id(42);Coordinator->HandleUse(Old);TestTrue(TEXT("Confirmed refusal resolves gate"),Coordinator->UseRequest.empty());
 TestEqual(TEXT("Refusal preserves assignment"),Coordinator->QuickSlots.Definitions[0],20001);
 Coordinator->ResetSession();TestEqual(TEXT("Disconnect clears preference memory"),Coordinator->QuickSlots.Definitions[0],0);
 TestTrue(TEXT("Disconnect removes namespace"),Coordinator->QuickIdentity.IsEmpty());
 Coordinator->HandleUse(Old);TestFalse(TEXT("Old response cannot restore disconnected capability"),Coordinator->bUseEnabled);
 return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuickSlotInputTest,"S1.QuickSlots.InputStarted",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuickSlotInputTest::RunTest(const FString& Parameters)
{
 auto* Context=LoadObject<UInputMappingContext>(nullptr,TEXT("/Game/Blueprints/Input/IMC_Default.IMC_Default"));
 if(!TestNotNull(TEXT("Existing real mapping context"),Context))return false;
 const TArray<FKey> Keys={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::NumPadOne,EKeys::NumPadTwo,EKeys::NumPadThree,EKeys::NumPadFour};
 for(const auto& Key:Keys)TestFalse(*FString::Printf(TEXT("No existing conflict: %s"),*Key.ToString()),
  Context->GetMappings().ContainsByPredicate([Key](const auto& Mapping){return Mapping.Key==Key;}));
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Pawn=World->SpawnActor<AS1MyPlayer>();
 if(TestNotNull(TEXT("Native pawn"),Pawn))
 {
  Pawn->DefaultMappingContext=Context;Pawn->EnsureQuickSlotInput();
  TestEqual(TEXT("Four actions"),Pawn->QuickSlotActions.Num(),4);TestEqual(TEXT("Eight key mappings"),Pawn->QuickSlotContext->GetMappings().Num(),8);
  auto* Input=NewObject<UEnhancedPlayerInput>();auto* Component=NewObject<UEnhancedInputComponent>();
  int32 Started=0;Component->BindActionValueLambda(Pawn->QuickSlotActions[0],ETriggerEvent::Started,[&Started](const FInputActionValue&){++Started;});
  TArray<UInputComponent*> Stack={Component};
  for(int32 Frame=0;Frame<20;++Frame){Input->InjectInputForAction(Pawn->QuickSlotActions[0],FInputActionValue(true));Input->ProcessInputStack(Stack,0.016f,false);}
  TestEqual(TEXT("Actual Enhanced Input held action starts once"),Started,1);
  Input->InjectInputForAction(Pawn->QuickSlotActions[0],FInputActionValue(false));Input->ProcessInputStack(Stack,0.016f,false);
  Input->InjectInputForAction(Pawn->QuickSlotActions[0],FInputActionValue(true));Input->ProcessInputStack(Stack,0.016f,false);
  TestEqual(TEXT("Release and repress starts again"),Started,2);
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
