#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1QuickSlotWidget.h"
#include "Loot/S1InventorySubsystem.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1QuickSlotWidgetBlueprintTest,"S1.QuickSlots.WidgetBlueprintView",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1QuickSlotWidgetBlueprintTest::RunTest(const FString& Parameters)
{
 auto Class=LoadClass<US1QuickSlotWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1QuickSlots.WBP_S1QuickSlots_C"));
 if(!TestNotNull(TEXT("Real saved Widget Blueprint class"),Class))return false;
 TestTrue(TEXT("Existing passive parent"),Class->IsChildOf(US1QuickSlotWidget::StaticClass()));
 auto* Instance=NewObject<UGameInstance>();auto* Coordinator=NewObject<US1InventorySubsystem>(Instance);
 TestTrue(TEXT("Subsystem uses cooked-reference blueprint"),Coordinator->GetQuickSlotWidgetClass().Get()==Class);
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("Transient presentation world"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* View=CreateWidget<US1QuickSlotWidget>(World,Class);
 if(TestNotNull(TEXT("Actual UMG instance"),View))
 {
  View->TakeWidget();
  TestNotNull(TEXT("Blueprint designer root exists"),View->WidgetTree->RootWidget.Get());
  UImage* Images[4]={};UTextBlock* Counts[4]={};UTextBlock* States[4]={};
  bool bAllBindings=true;
  for(int32 I=0;I<4;++I)
  {
   Images[I]=Cast<UImage>(View->GetWidgetFromName(FName(*FString::Printf(TEXT("Icon%d"),I+1))));
   Counts[I]=Cast<UTextBlock>(View->GetWidgetFromName(FName(*FString::Printf(TEXT("Count%d"),I+1))));
   States[I]=Cast<UTextBlock>(View->GetWidgetFromName(FName(*FString::Printf(TEXT("State%d"),I+1))));
   TestNotNull(TEXT("Required image binding"),Images[I]);TestNotNull(TEXT("Required count binding"),Counts[I]);TestNotNull(TEXT("Required state binding"),States[I]);
   bAllBindings=bAllBindings&&Images[I]&&Counts[I]&&States[I];
  }
  if(bAllBindings)
  {
   // Styles are owned by the designer and must survive model updates.
   auto Font=Counts[0]->GetFont();Font.Size=23;Counts[0]->SetFont(Font);
   auto Brush=Images[0]->GetBrush();Brush.ImageSize=FVector2D(61,55);Images[0]->SetBrush(Brush);
   View->SetView({20001,0,20001,20001},5,TEXT("사용 가능"),true);
   TestEqual(TEXT("Live numeric count reaches bound UMG"),Counts[0]->GetText().ToString(),FString(TEXT("5")));
   TestEqual(TEXT("Fourth slot shared stock"),Counts[3]->GetText().ToString(),FString(TEXT("5")));
   TestEqual(TEXT("Empty assignment view"),Counts[1]->GetText().ToString(),FString());
   TestTrue(TEXT("Empty slot icon hidden"),Images[1]->GetVisibility()==ESlateVisibility::Hidden);
   TestTrue(TEXT("Runtime icon uses existing texture"),Images[0]->GetBrush().GetResourceObject()==GetDefault<US1PotionPresentationSettings>()->Icon.TryLoad());
   TestEqual(TEXT("Designer font not overwritten"),Counts[0]->GetFont().Size,23.f);
   TestTrue(TEXT("Designer image size not overwritten"),Images[0]->GetBrush().ImageSize==FVector2D(61,55));
   View->SetView({20001,0,20001,20001},0,TEXT("보유 없음"),false);
   TestEqual(TEXT("Zero keeps registered slot"),Counts[0]->GetText().ToString(),FString(TEXT("0")));
   TestEqual(TEXT("Unavailable icon dimmed"),Images[0]->GetRenderOpacity(),0.4f);
   View->SetView({20001,0,20001,20001},7,TEXT("미확정 (I)"),false);
   TestEqual(TEXT("Unknown state reaches view"),States[3]->GetText().ToString(),FString(TEXT("미확정 (I)")));
  }
  // Layout containers/names/anchors are designer-owned, not the model binding contract.
  auto* LayoutRoot=View->WidgetTree->RootWidget.Get();
  if(TestNotNull(TEXT("Designer layout root"),LayoutRoot))
  {
   auto Transform=LayoutRoot->GetRenderTransform();
   Transform.Translation=FVector2D(17,-13);Transform.Angle=8.f;
   LayoutRoot->SetRenderTransform(Transform);
   View->SetView({20001,0,20001,20001},8,TEXT("사용 가능"),true);
   TestTrue(TEXT("Model update preserves designer position"),LayoutRoot->GetRenderTransform().Translation==Transform.Translation);
   TestEqual(TEXT("Model update preserves designer rotation"),LayoutRoot->GetRenderTransform().Angle,Transform.Angle);
  }
  View->RemoveFromParent();
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
