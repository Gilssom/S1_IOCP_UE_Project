#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1InventorySubsystem.h"
#include "UI/S1InventoryWidget.h"
#include "UI/S1InventorySplitWidget.h"
#include "Components/SpinBox.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InventoryStackCoordinatorTest,"S1.Inventory.StackCoordinator",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InventoryStackCoordinatorTest::RunTest(const FString&)
{
 auto* GI=NewObject<UGameInstance>();auto* C=NewObject<US1InventorySubsystem>(GI);auto* View=NewObject<US1InventoryWidget>(GI);
 C->BeginSession(42,true,true,true,true);C->InventoryWidget=View;
 Protocol::S_INVENTORY_SNAPSHOT P;P.set_character_id(42);P.set_success(true);P.set_inventory_version(3);
 auto* E=P.add_entries();E->set_item_instance_id(8);E->set_item_definition_id(20001);E->set_slot_index(0);E->set_count(30);
 E=P.add_entries();E->set_item_instance_id(9);E->set_item_definition_id(20001);E->set_slot_index(1);E->set_count(80);C->State.Apply(P);
 TestTrue(TEXT("Same-definition explicit swap allowed"),C->ValidateStack(1,8,0,1,9,3,0));
 TestTrue(TEXT("Partial merge allowed"),C->ValidateStack(2,8,0,1,9,3,0));
 TestTrue(TEXT("Split into empty slot"),C->ValidateStack(3,8,0,2,0,3,4));
 TestFalse(TEXT("Split zero"),C->ValidateStack(3,8,0,2,0,3,0));
 TestFalse(TEXT("Split whole stack"),C->ValidateStack(3,8,0,2,0,3,30));
 TestFalse(TEXT("Split occupied"),C->ValidateStack(3,8,0,1,9,3,4));
 TestFalse(TEXT("Stale drag"),C->ValidateStack(1,8,0,1,9,2,0));
 TestFalse(TEXT("Foreign item"),C->ValidateStack(2,99,0,1,9,3,0));
 TestFalse(TEXT("Wrong target identity"),C->ValidateStack(2,8,0,1,88,3,0));
 TestFalse(TEXT("Locked destination"),C->ValidateStack(3,8,0,20,0,3,1));
 C->UseRequest=std::string(16,'u');TestFalse(TEXT("Pending consumption blocks swap"),C->ValidateStack(1,8,0,1,9,3,0));C->UseRequest.clear();
 C->StackRequest=std::string(16,'s');C->StackItem=8;C->StackVersion=3;C->StackSource=0;C->StackDestination=1;C->StackAction=2;
 TestFalse(TEXT("Stack blocks move"),C->RequestMoveItem(8,0,2,3,View));TestFalse(TEXT("Stack blocks quick-slot registration"),C->RegisterQuickSlotDrag(0,8,View));
 Protocol::S_INVENTORY_STACK_ACTION_RESULT R;R.set_request_id(std::string(16,'x'));R.set_character_id(42);R.set_source_item_id(8);
 R.set_action(2);R.set_source_slot(0);R.set_destination_slot(1);R.set_decision(1);R.set_inventory_version(4);R.set_source_count(11);R.set_destination_count(99);
 C->HandleStack(R);TestFalse(TEXT("Old response ignored"),C->bStackReconciling);
 R.set_request_id(C->StackRequest);C->HandleStack(R);TestTrue(TEXT("Accepted merge starts snapshot reconciliation"),C->bStackReconciling);
 TestEqual(TEXT("No optimistic amount change"),C->State.GetCount(),int64(110));TestFalse(TEXT("Pre-commit state locked"),C->State.ReadyForUse());
 C->bQueryPending=true;C->QueryRequest=std::string(16,'q');P.set_request_id(C->QueryRequest);C->HandleInventory(P);
 TestTrue(TEXT("Old snapshot does not unlock"),!C->StackRequest.empty());
 P.set_inventory_version(4);P.mutable_entries(0)->set_count(11);P.mutable_entries(1)->set_count(99);C->bQueryPending=true;C->HandleInventory(P);
 TestTrue(TEXT("New snapshot unlocks"),C->StackRequest.empty());TestEqual(TEXT("Total preserved"),C->State.GetCount(),int64(110));
 TestFalse(TEXT("Full destination cannot merge"),C->ValidateStack(2,8,0,1,9,4,0));TestTrue(TEXT("Full destination still permits explicit swap"),C->ValidateStack(1,8,0,1,9,4,0));
 C->StackRequest=std::string(16,'n');C->StackItem=8;C->StackVersion=4;R.set_request_id(std::string(16,'s'));C->HandleStack(R);
 TestTrue(TEXT("Late response leaves newer request"),C->StackRequest==std::string(16,'n'));
 R.set_request_id(C->StackRequest);R.set_decision(3);R.set_reason(8);C->HandleStack(R);
 TestTrue(TEXT("Unknown retains request and only reconciles"),C->bStackReconciling&&!C->StackRequest.empty());
 C->InventoryWidget=nullptr;C->ResetSession();TestTrue(TEXT("Session reset clears stack"),C->StackRequest.empty());TestFalse(TEXT("Capability reset"),C->bStackEnabled);
 C->BeginSession(42,true,true,true);C->InventoryWidget=View;TestFalse(TEXT("Legacy server disables stacks"),C->ValidateStack(3,8,0,2,0,4,1));C->InventoryWidget=nullptr;C->ResetSession();
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1InventorySplitWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1InventorySplitDialog.WBP_S1InventorySplitDialog_C"));
 if(TestNotNull(TEXT("Saved editable split WBP"),Class))
 {
  TestTrue(TEXT("Hard cooker class reference matches saved dialog"),C->SplitWidgetClass.Get()==Class);
  auto* Dialog=CreateWidget<US1InventorySplitWidget>(World,Class);Dialog->SetSourceCount(10);Dialog->TakeWidget();
  if(TestNotNull(TEXT("Amount binding"),Dialog->AmountInput.Get()))
  {
   TestEqual(TEXT("Initial half amount"),Dialog->AmountInput->GetValue(),5.f);uint32 Amount=0;int Calls=0,Cancels=0;
   Dialog->OnConfirm.BindLambda([&](uint32 N){Amount=N;++Calls;});Dialog->OnCancel.BindLambda([&]{++Cancels;});
   Dialog->AmountInput->SetValue(4);Dialog->ConfirmClick();TestEqual(TEXT("Exact selected quantity"),Amount,uint32(4));
   Dialog->AmountInput->SetValue(4.5f);Dialog->ConfirmClick();TestEqual(TEXT("Fractional input rejected"),Calls,1);
   const FKeyEvent Escape(EKeys::Escape,FModifierKeysState(),0,false,0,0);Dialog->NativeOnPreviewKeyDown(FGeometry(),Escape);TestEqual(TEXT("Escape cancels"),Cancels,1);
   Dialog->OnConfirm.Unbind();Dialog->OnCancel.Unbind();Dialog->NativeDestruct();Dialog->RemoveFromParent();
  }
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return !HasAnyErrors();
}
#endif
