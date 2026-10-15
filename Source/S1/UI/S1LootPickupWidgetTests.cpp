#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1LootPickupWidget.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1LootPickupWidgetTest,"S1.Loot.PickupWidget",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1LootPickupWidgetTest::RunTest(const FString&)
{
 auto* GI=NewObject<UGameInstance>();auto* View=NewObject<US1LootPickupWidget>(GI);
 View->KeyText=NewObject<UTextBlock>(View);View->ItemText=NewObject<UTextBlock>(View);View->StatusText=NewObject<UTextBlock>(View);
 View->SetPickupView(FText::FromString(TEXT("회복 포션")),12,false);
 TestEqual(TEXT("G input label"),View->KeyText->GetText().ToString(),FString(TEXT("G")));
 TestTrue(TEXT("Whole selected count"),View->ItemText->GetText().ToString().Contains(TEXT("12")));
 TestEqual(TEXT("Ready label"),View->StatusText->GetText().ToString(),FString(TEXT("획득")));
 View->SetPickupView(FText::FromString(TEXT("회복 포션")),12,true);
 TestEqual(TEXT("Pending label"),View->StatusText->GetText().ToString(),FString(TEXT("획득 처리 중…")));
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1LootPickupWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1LootPickupPrompt.WBP_S1LootPickupPrompt_C"));
 if(TestNotNull(TEXT("Saved editable WBP class"),Class))
 {
  auto* Saved=CreateWidget<US1LootPickupWidget>(World,Class);Saved->SetPickupView(FText::FromString(TEXT("회복 포션")),7,true);Saved->TakeWidget();
  TestNotNull(TEXT("Key binding"),Saved->KeyText.Get());TestNotNull(TEXT("Item binding"),Saved->ItemText.Get());TestNotNull(TEXT("Status binding"),Saved->StatusText.Get());
  TestFalse(TEXT("Does not take keyboard focus"),Saved->IsFocusable());
  TestEqual(TEXT("Does not intercept UI clicks"),Saved->GetVisibility(),ESlateVisibility::HitTestInvisible);
  if(Saved->ItemText)TestTrue(TEXT("View survives construct"),Saved->ItemText->GetText().ToString().Contains(TEXT("7")));
  Saved->RemoveFromParent();
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return !HasAnyErrors();
}
#endif
