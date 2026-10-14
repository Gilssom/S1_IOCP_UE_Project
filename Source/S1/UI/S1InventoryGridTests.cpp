#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1InventoryWidget.h"
#include "UI/S1InventorySlotWidget.h"
#include "UI/S1InventoryDragOperation.h"
#include "Loot/S1InventorySubsystem.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InventoryGridTest,"S1.Inventory.GridAndDrag",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InventoryGridTest::RunTest(const FString& Parameters)
{
 auto* Class=LoadClass<US1InventoryWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1InventoryGrid.WBP_S1InventoryGrid_C"));
 if(!TestNotNull(TEXT("Saved grid class"),Class))return false;
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* View=CreateWidget<US1InventoryWidget>(World,Class);
 if(TestNotNull(TEXT("Actual grid view"),View))
 {
  View->TakeWidget();View->SetUseView({{8,2,0},{9,5,19}},true,false);
  TestEqual(TEXT("80 visible cells"),View->GridSlots.Num(),80);
  TestNotNull(TEXT("Grid binding"),View->InventoryGrid.Get());
  TestFalse(TEXT("No registration/use/clear buttons in grid"),View->UseButton||View->Register1||View->Clear1);
  if(View->GridSlots.Num()==80)
  {
   auto* First=View->GridSlots[0].Get();First->TakeWidget();
   auto* Last=View->GridSlots[79].Get();Last->TakeWidget();
   auto* Layout=Cast<UUniformGridSlot>(Last->Slot);
   if(TestNotNull(TEXT("Last cell grid slot"),Layout))
   {TestEqual(TEXT("10 rows"),Layout->GetRow(),9);TestEqual(TEXT("8 columns"),Layout->GetColumn(),7);}
   TestEqual(TEXT("Server instance mapped to correct slot"),View->GridSlots[19]->GetItemId(),uint64(9));
   TestTrue(TEXT("Future capacity is locked"),Last->bLocked);
   TestTrue(TEXT("No hidden phantom stock in future slots"),Last->GetItemId()==0);
   TestEqual(TEXT("Count bound"),First->CountText->GetText().ToString(),FString(TEXT("2")));
   auto Font=First->CountText->GetFont();Font.Size=17;First->CountText->SetFont(Font);
   uint64 Used=0;int32 UseCalls=0;
   View->OnUseRequested.BindLambda([&](uint64 Id){Used=Id;++UseCalls;});
   const FPointerEvent Left(0,FVector2D(10,10),FVector2D(10,10),TSet<FKey>{EKeys::LeftMouseButton},EKeys::LeftMouseButton,0,FModifierKeysState());
   First->NativeOnMouseButtonDoubleClick(FGeometry(),Left);
   TestEqual(TEXT("Double click sends one use intent"),UseCalls,1);TestEqual(TEXT("Use exact instance"),Used,uint64(8));
   UDragDropOperation* Op=nullptr;First->NativeOnDragDetected(FGeometry(),Left,Op);
   auto* Drag=Cast<US1InventoryDragOperation>(Op);
   if(TestNotNull(TEXT("Typed drag payload"),Drag))
   {TestTrue(TEXT("Correct source window"),Drag->SourceWindow.Get()==View);TestEqual(TEXT("Drag exact ID"),Drag->ItemId,uint64(8));}
   View->SetUseView({{8,1,0},{9,5,19}},false,false);
   TestTrue(TEXT("Stable cell across snapshots"),View->GridSlots[0]==First);
   TestEqual(TEXT("Designer font preserved"),First->CountText->GetFont().Size,17.f);
   First->NativeOnMouseButtonDoubleClick(FGeometry(),Left);TestEqual(TEXT("Busy blocks use"),UseCalls,1);
   First->NativeOnDragDetected(FGeometry(),Left,Op);TestTrue(TEXT("Busy blocks drag"),Op==nullptr);
   View->SetUseView({},true,true);
   const FKeyEvent Retry(EKeys::R,FModifierKeysState(),0,false,0,0);
   View->NativeOnPreviewKeyDown(FGeometry(),Retry);
   TestEqual(TEXT("Explicit original-request retry available without button"),UseCalls,2);
   TestEqual(TEXT("Retry does not invent instance"),Used,uint64(0));
   Last->NativeOnDragDetected(FGeometry(),Left,Op);TestTrue(TEXT("Empty/locked does not drag"),Op==nullptr);
   TestEqual(TEXT("No deletion on cosmetic drag"),View->GridSlots.Num(),80);
  }
  auto* GI=NewObject<UGameInstance>();auto* Coordinator=NewObject<US1InventorySubsystem>(GI);
  Coordinator->BeginSession(42,true,true);Coordinator->InventoryWidget=View;
  Protocol::S_INVENTORY_SNAPSHOT Snapshot;Snapshot.set_character_id(42);Snapshot.set_success(true);Snapshot.set_inventory_version(1);
  auto* E=Snapshot.add_entries();E->set_item_instance_id(8);E->set_item_definition_id(20001);E->set_slot_index(0);E->set_count(2);
  Coordinator->State.Apply(Snapshot);
  TestFalse(TEXT("Foreign source window rejected"),Coordinator->RegisterQuickSlotDrag(0,8,nullptr));
  TestFalse(TEXT("Foreign item rejected"),Coordinator->RegisterQuickSlotDrag(0,999,View));
  for(int32 I=0;I<4;++I)
  {TestTrue(TEXT("Four drag destinations"),Coordinator->RegisterQuickSlotDrag(I,8,View));TestEqual(TEXT("Definition assignment"),Coordinator->QuickSlots.Definitions[I],20001);}
  TestEqual(TEXT("Registration never consumes inventory"),Coordinator->State.GetCount(),int64(2));
  Coordinator->ClearQuickSlot(1);TestEqual(TEXT("Clear immediate without confirmation"),Coordinator->QuickSlots.Definitions[1],0);
  Coordinator->UseRequest=std::string(16,'p');TestFalse(TEXT("Pending common gate blocks drag"),Coordinator->RegisterQuickSlotDrag(1,8,View));
  Coordinator->UseRequest.clear();Coordinator->InventoryWidget=nullptr;
  TestFalse(TEXT("Closed window payload rejected"),Coordinator->RegisterQuickSlotDrag(1,8,View));
  View->NativeDestruct();TestEqual(TEXT("Rows unbound on close"),View->GridSlots.Num(),0);
  View->RemoveFromParent();
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
