#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1InventoryWidget.h"
#include "UI/S1InventoryStackWidget.h"
#include "Loot/S1InventorySubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1InventoryWidgetBlueprintTest,"S1.Inventory.WidgetBlueprintIntents",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1InventoryWidgetBlueprintTest::RunTest(const FString& Parameters)
{
 auto MainClass=LoadClass<US1InventoryWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1Inventory.WBP_S1Inventory_C"));
 auto RowClass=LoadClass<US1InventoryStackWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1InventoryStackRow.WBP_S1InventoryStackRow_C"));
 if(!TestNotNull(TEXT("Saved window class"),MainClass)||!TestNotNull(TEXT("Saved row class"),RowClass))return false;
 auto* Instance=NewObject<UGameInstance>();auto* Coordinator=NewObject<US1InventorySubsystem>(Instance);
 auto GridClass=LoadClass<US1InventoryWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1InventoryGrid.WBP_S1InventoryGrid_C"));
 TestTrue(TEXT("Coordinator selects new grid; legacy intent contract still valid"),GridClass&&Coordinator->GetInventoryWidgetClass().Get()==GridClass);
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("Transient presentation world"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* View=CreateWidget<US1InventoryWidget>(World,MainClass);
 if(TestNotNull(TEXT("Actual saved UMG window instance"),View))
 {
  View->TakeWidget();
  TestTrue(TEXT("Saved row template reference"),View->GetStackEntryClass().Get()==RowClass);
  const bool bBound=View->PotionIcon&&View->QuantityLabel&&View->StatusLabel&&View->SelectionLabel&&View->UseLabel&&View->StackEntryList&&
   View->UseButton&&View->CloseButton&&View->Register1&&View->Register2&&View->Register3&&View->Register4&&
   View->Clear1&&View->Clear2&&View->Clear3&&View->Clear4;
  if(TestTrue(TEXT("All 16 required window bindings"),bBound))
  {
   auto Font=View->QuantityLabel->GetFont();Font.Size=22;View->QuantityLabel->SetFont(Font);
   auto Brush=View->PotionIcon->GetBrush();Brush.ImageSize=FVector2D(61,55);View->PotionIcon->SetBrush(Brush);
   View->SetPotionView(7,TEXT("서버 확정 수량"));View->SetUseView({{8,2,0},{9,5,1}},true,false);
   TestEqual(TEXT("Actual authoritative total presentation"),View->QuantityLabel->GetText().ToString(),FString(TEXT("회복 포션 x7")));
   TestEqual(TEXT("Designer font preserved"),View->QuantityLabel->GetFont().Size,22.f);
   TestTrue(TEXT("Designer image size preserved"),View->PotionIcon->GetBrush().ImageSize==FVector2D(61,55));
   TestEqual(TEXT("Two rows created"),View->StackEntryList->GetChildrenCount(),2);
   TestFalse(TEXT("No selection means use disabled"),View->UseButton->GetIsEnabled());
   auto* First=View->StackEntryList->GetChildrenCount()>0?Cast<US1InventoryStackWidget>(View->StackEntryList->GetChildAt(0)):nullptr;
   if(TestNotNull(TEXT("Actual row instance"),First))
   {
    First->TakeWidget();
    auto* Button=Cast<UButton>(First->GetWidgetFromName(TEXT("SelectButton")));
    auto* Label=Cast<UTextBlock>(First->GetWidgetFromName(TEXT("EntryText")));
    if(TestNotNull(TEXT("Row button binding"),Button)&&TestNotNull(TEXT("Row label binding"),Label))
    {
     uint64 UsedItem=0,RegisteredItem=0;int32 UseCalls=0,RegisterSlot=-1,ClearSlot=-1,Closed=0;
     View->OnUseRequested.BindLambda([&](uint64 Id){UsedItem=Id;++UseCalls;});
     View->OnRegisterRequested.BindLambda([&](int32 Slot,uint64 Id){RegisterSlot=Slot;RegisteredItem=Id;});
     View->OnClearRequested.BindLambda([&](int32 Slot){ClearSlot=Slot;});
     View->OnCloseRequested.BindLambda([&]{++Closed;});
     Button->OnClicked.Broadcast();
     TestEqual(TEXT("Row intent selects current instance"),View->SelectedItem,uint64(8));
     TestTrue(TEXT("Selection indicated"),Label->GetText().ToString().Contains(TEXT("▶")));
     TestTrue(TEXT("Use enabled after selection"),View->UseButton->GetIsEnabled());
     View->SelectEntry(999);TestEqual(TEXT("Foreign instance ignored"),View->SelectedItem,uint64(8));
     View->NativeConstruct(); // AddUniqueDynamic must not double-bind on reconstruction.
     View->UseButton->OnClicked.Broadcast();
     TestEqual(TEXT("Exactly one use intent"),UseCalls,1);TestEqual(TEXT("Selected use identifier"),UsedItem,uint64(8));
     UButton* RegisterButtons[4]={View->Register1,View->Register2,View->Register3,View->Register4};
     UButton* ClearButtons[4]={View->Clear1,View->Clear2,View->Clear3,View->Clear4};
     for(int32 I=0;I<4;++I)
     {RegisterButtons[I]->OnClicked.Broadcast();TestEqual(TEXT("Registration slot intent"),RegisterSlot,I);
      TestEqual(TEXT("Registration selected stack intent"),RegisteredItem,uint64(8));
      ClearButtons[I]->OnClicked.Broadcast();TestEqual(TEXT("Clear slot intent"),ClearSlot,I);}
     View->SetUseView({{8,1,0},{9,5,1}},false,false);
     TestTrue(TEXT("Same identifier row retained"),View->StackEntryList->GetChildAt(0)==First);
     TestTrue(TEXT("Server count update reaches same row"),Label->GetText().ToString().Contains(TEXT("x1")));
     View->UseButton->OnClicked.Broadcast();TestEqual(TEXT("Processing gate blocks even direct delegate invocation"),UseCalls,1);
     View->SetUseView({{8,1,0},{9,5,1}},true,true);
     TestFalse(TEXT("Unknown disables selection"),Button->GetIsEnabled());
     TestFalse(TEXT("Unknown disables registration"),View->Register1->GetIsEnabled());
     View->SelectEntry(9);TestEqual(TEXT("Unknown cannot switch selection"),View->SelectedItem,uint64(8));
     TestTrue(TEXT("Explicit original-request retry label"),View->UseLabel->GetText().ToString().Contains(TEXT("재확인")));
     View->SetUseView({},true,true);
     TestEqual(TEXT("Removed selection cleared"),View->SelectedItem,uint64(0));
     TestEqual(TEXT("No phantom rows after removal"),View->StackEntryList->GetChildrenCount(),0);
     Button->OnClicked.Broadcast();TestEqual(TEXT("Detached row intent unbound"),View->SelectedItem,uint64(0));
     View->UseButton->OnClicked.Broadcast();TestEqual(TEXT("Unknown retry remains possible without old visible stack"),UseCalls,2);
     TestEqual(TEXT("No invented item ID for retry"),UsedItem,uint64(0));
     View->SetUseView({},true,false);TestFalse(TEXT("Zero items ordinary use disabled"),View->UseButton->GetIsEnabled());
     View->CloseButton->OnClicked.Broadcast();TestEqual(TEXT("Close button intent"),Closed,1);
     const FKeyEvent IKey(EKeys::I,FModifierKeysState(),0,false,0,0);
     const FKeyEvent Escape(EKeys::Escape,FModifierKeysState(),0,false,0,0);
     TestTrue(TEXT("Preview I handled before child focus"),View->NativeOnPreviewKeyDown(FGeometry(),IKey).IsEventHandled());
     TestTrue(TEXT("Preview Esc handled before child focus"),View->NativeOnPreviewKeyDown(FGeometry(),Escape).IsEventHandled());
     TestEqual(TEXT("Exactly one close intent per key"),Closed,3);
    }
   }
  }
  View->RemoveFromParent();
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
