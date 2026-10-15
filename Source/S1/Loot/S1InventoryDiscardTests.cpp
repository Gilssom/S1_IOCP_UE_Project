#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Loot/S1InventorySubsystem.h"
#include "UI/S1InventoryWidget.h"
#include "UI/S1InventoryDiscardWidget.h"
#include "UI/S1InventoryDropZoneWidget.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InventoryDiscardTest,"S1.Inventory.Discard",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InventoryDiscardTest::RunTest(const FString&)
{
 TestFalse(TEXT("Unarranged panel fails closed"),US1InventoryDropZoneWidget::IsOutsideArrangedPanel(FGeometry(),FVector2D(100,100)));
 const auto PanelGeometry=FGeometry::MakeRoot(FVector2D(420,600),FSlateLayoutTransform(FVector2D(100,100)));
 TestFalse(TEXT("Panel interior/gaps never discard"),US1InventoryDropZoneWidget::IsOutsideArrangedPanel(PanelGeometry,FVector2D(200,200)));
 TestTrue(TEXT("Outside arranged panel candidate"),US1InventoryDropZoneWidget::IsOutsideArrangedPanel(PanelGeometry,FVector2D(600,200)));
 auto* GI=NewObject<UGameInstance>();auto* C=NewObject<US1InventorySubsystem>(GI);auto* View=NewObject<US1InventoryWidget>(GI);
 C->BeginSession(42,true,true,true,true,true);C->InventoryWidget=View;
 Protocol::S_INVENTORY_SNAPSHOT P;P.set_character_id(42);P.set_success(true);P.set_inventory_version(3);
 auto* E=P.add_entries();E->set_item_instance_id(8);E->set_item_definition_id(20001);E->set_slot_index(0);E->set_count(10);C->State.Apply(P);
 TestTrue(TEXT("Partial allowed"),C->ValidateDiscard(8,0,3,3));TestTrue(TEXT("Whole allowed"),C->ValidateDiscard(8,0,3,10));
 TestFalse(TEXT("Zero rejected"),C->ValidateDiscard(8,0,3,0));TestFalse(TEXT("Excess rejected"),C->ValidateDiscard(8,0,3,11));
 TestFalse(TEXT("Foreign rejected"),C->ValidateDiscard(9,0,3,1));TestFalse(TEXT("Locked rejected"),C->ValidateDiscard(8,20,3,1));TestFalse(TEXT("Stale rejected"),C->ValidateDiscard(8,0,2,1));
 C->UseRequest=std::string(16,'u');TestFalse(TEXT("Pending use blocks"),C->ValidateDiscard(8,0,3,1));C->UseRequest.clear();
 C->DiscardRequest=std::string(16,'d');C->DiscardItem=8;C->DiscardSource=0;C->DiscardVersion=3;C->DiscardAmount=3;C->DiscardOriginalCount=10;
 TestFalse(TEXT("Pending discard blocks movement"),C->RequestMoveItem(8,0,1,3,View));TestFalse(TEXT("Pending discard blocks split"),C->ValidateStack(3,8,0,1,0,3,1));
 Protocol::S_PLAYER_FIELD_DROP_RESULT R;R.set_request_id(std::string(16,'x'));R.set_character_id(42);R.set_source_item_id(8);R.set_source_slot(0);R.set_requested_amount(3);R.set_decision(1);R.set_inventory_version(4);R.set_remaining_count(7);R.set_drop_id(std::string(16,'f'));
 C->HandleDiscard(R);TestFalse(TEXT("Old response ignored"),C->bDiscardReconciling);
 R.set_request_id(C->DiscardRequest);R.set_remaining_count(6);C->HandleDiscard(R);TestFalse(TEXT("Invalid remaining ignored"),C->bDiscardReconciling);
 R.set_remaining_count(7);C->HandleDiscard(R);TestTrue(TEXT("Committed reconciles"),C->bDiscardReconciling);TestEqual(TEXT("No optimistic decrement"),C->State.GetCount(),int64(10));
 C->bQueryPending=true;C->QueryRequest=std::string(16,'q');P.set_request_id(C->QueryRequest);C->HandleInventory(P);TestTrue(TEXT("Precommit snapshot cannot unlock"),!C->DiscardRequest.empty());
 P.set_inventory_version(4);P.mutable_entries(0)->set_count(7);C->bQueryPending=true;C->HandleInventory(P);TestTrue(TEXT("Confirmed snapshot unlocks"),C->DiscardRequest.empty());TestEqual(TEXT("Server count applied"),C->State.GetCount(),int64(7));
 auto* Dialog=NewObject<US1InventoryDiscardWidget>(GI);Dialog->SetSourceCount(10);uint32 Amount=0;int Calls=0,Cancels=0;
 Dialog->OnConfirm.BindLambda([&](uint32 N){Amount=N;++Calls;});Dialog->OnCancel.BindLambda([&]{++Cancels;});
 Dialog->ConfirmClick();TestEqual(TEXT("Quantity next is not deletion"),Calls,0);TestTrue(TEXT("Next stage final"),Dialog->bFinal);
 const FKeyEvent Enter(EKeys::Enter,FModifierKeysState(),0,false,0,0);Dialog->NativeOnPreviewKeyDown(FGeometry(),Enter);TestEqual(TEXT("Enter cannot auto-confirm"),Calls,0);
 Dialog->ConfirmClick();Dialog->ConfirmClick();TestEqual(TEXT("Final confirmation once"),Calls,1);TestEqual(TEXT("Safe default1"),Amount,uint32(1));
 Dialog->SetSourceCount(1);TestTrue(TEXT("Single goes straight to final"),Dialog->bFinal);const FKeyEvent Escape(EKeys::Escape,FModifierKeysState(),0,false,0,0);Dialog->NativeOnPreviewKeyDown(FGeometry(),Escape);TestEqual(TEXT("Escape cancels"),Cancels,1);
 C->DiscardRequest=std::string(16,'n');C->DiscardItem=8;C->DiscardVersion=4;C->DiscardSource=0;C->DiscardAmount=1;C->DiscardOriginalCount=7;C->bDiscardReconciling=false;
 R.set_request_id(std::string(16,'d'));C->HandleDiscard(R);TestTrue(TEXT("Late response keeps newer request"),C->DiscardRequest==std::string(16,'n'));TestFalse(TEXT("Late response cannot start reconciliation"),C->bDiscardReconciling);
 R.set_request_id(C->DiscardRequest);R.set_requested_amount(1);R.set_decision(3);R.set_reason(6);C->HandleDiscard(R);TestTrue(TEXT("Unknown keeps request and reconciles without resend"),C->bDiscardReconciling&&!C->DiscardRequest.empty());
 C->InventoryWidget=nullptr;C->ResetSession();TestTrue(TEXT("Session clears pending"),C->DiscardRequest.empty());TestFalse(TEXT("Capability reset"),C->bDiscardEnabled);
 C->BeginSession(42,true,true,true,true);C->InventoryWidget=View;C->State.Apply(P);TestFalse(TEXT("Legacy capability disables"),C->ValidateDiscard(8,0,4,1));C->InventoryWidget=nullptr;C->ResetSession();
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1InventoryDiscardWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1InventoryDiscardDialog.WBP_S1InventoryDiscardDialog_C"));
 if(TestNotNull(TEXT("Saved discard WBP"),Class))
 {
  TestTrue(TEXT("Hard cooker reference matches saved WBP"),C->DiscardWidgetClass.Get()==Class);
  auto* Saved=CreateWidget<US1InventoryDiscardWidget>(World,Class);Saved->SetSourceCount(10);Saved->TakeWidget();
  if(TestNotNull(TEXT("Amount binding"),Saved->AmountInput.Get())&&TestNotNull(TEXT("Message binding"),Saved->MessageText.Get())&&TestNotNull(TEXT("Confirm label binding"),Saved->ConfirmText.Get())&&TestNotNull(TEXT("Confirm button binding"),Saved->ConfirmButton.Get())&&TestNotNull(TEXT("Cancel button binding"),Saved->CancelButton.Get()))
  {
   int Submitted=0;uint32 Chosen=0;Saved->OnConfirm.BindLambda([&](uint32 N){Chosen=N;++Submitted;});
   TestEqual(TEXT("Saved initial quantity1"),Saved->AmountInput->GetValue(),1.f);
   Saved->AmountInput->SetValue(3.5f);Saved->ConfirmClick();TestFalse(TEXT("Saved fraction rejected"),Saved->bFinal);
   Saved->AmountInput->SetValue(3);Saved->ConfirmClick();TestEqual(TEXT("Next does not submit saved view"),Submitted,0);
   TestEqual(TEXT("Quantity field hidden in final"),Saved->AmountInput->GetVisibility(),ESlateVisibility::Collapsed);
   TestEqual(TEXT("Final label changes"),Saved->ConfirmText->GetText().ToString(),FString(TEXT("예, 버리기")));
   TestTrue(TEXT("Final message contains selected quantity"),Saved->MessageText->GetText().ToString().Contains(TEXT("3개")));
   TestTrue(TEXT("Public field drop warning"),Saved->MessageText->GetText().ToString().Contains(TEXT("누구나")));
   TestFalse(TEXT("No permanent deletion warning"),Saved->MessageText->GetText().ToString().Contains(TEXT("영구 삭제")));
   Saved->ConfirmClick();Saved->ConfirmClick();TestEqual(TEXT("Saved final exactly one UI callback"),Submitted,1);TestEqual(TEXT("Saved exact quantity"),Chosen,uint32(3));
   Saved->OnConfirm.Unbind();Saved->NativeDestruct();Saved->RemoveFromParent();
  }
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
