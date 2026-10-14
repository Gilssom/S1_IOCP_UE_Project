#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1EntityHealthWidget.h"
#include "UI/S1EntityHealthComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1EntityHealthWidgetTest,"S1.UI.EntityHealth",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1EntityHealthWidgetTest::RunTest(const FString& Parameters)
{
 auto* Class=LoadClass<US1EntityHealthWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1EntityHealth.WBP_S1EntityHealth_C"));
 if(!TestNotNull(TEXT("Saved HP WBP uses native parent"),Class))return false;
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* A=CreateWidget<US1EntityHealthWidget>(World,Class);
 auto* B=CreateWidget<US1EntityHealthWidget>(World,Class);
 if(TestNotNull(TEXT("First entity view"),A)&&TestNotNull(TEXT("Second entity view"),B))
 {
  A->SetServerView(25,100,false,FText::FromString(TEXT("Knight")));
  A->TakeWidget();B->TakeWidget();
  const bool bBound=TestNotNull(TEXT("HealthBar binding"),A->HealthBar.Get())&&
   TestNotNull(TEXT("HealthText binding"),A->HealthText.Get())&&TestNotNull(TEXT("NameText binding"),A->NameText.Get());
  if(bBound)
  {
   TestEqual(TEXT("Snapshot before construction survives"),A->HealthBar->GetPercent(),.25f);
   auto Font=A->HealthText->GetFont();Font.Size=19;A->HealthText->SetFont(Font);
   A->SetServerView(75,100,false,FText::FromString(TEXT("Knight")));
   B->SetServerView(10,200,false,FText::FromString(TEXT("Dusk")));
   TestEqual(TEXT("Heal updated"),A->HealthBar->GetPercent(),.75f);
   TestEqual(TEXT("Other entity independent"),B->GetHealthFraction(),.05f);
   TestEqual(TEXT("Designer font survives"),A->HealthText->GetFont().Size,19.f);
   TestFalse(TEXT("Passive widget cannot take keyboard focus"),A->IsFocusable());
   TestTrue(TEXT("Passive view cannot intercept mouse"),A->GetVisibility()==ESlateVisibility::HitTestInvisible);
   A->SetServerView(0,100,true,FText::FromString(TEXT("Knight")));
   TestEqual(TEXT("Death fraction"),A->HealthBar->GetPercent(),0.f);
   TestTrue(TEXT("Death label"),A->HealthText->GetText().ToString().Contains(TEXT("DEAD")));
   A->SetServerView(150,100,false,FText::FromString(TEXT("Knight")));
   TestEqual(TEXT("Visual over-max clamped"),A->HealthBar->GetPercent(),1.f);
   A->SetServerView(0,0,false,FText::GetEmpty());
   TestEqual(TEXT("Unknown maximum safe"),A->HealthBar->GetPercent(),0.f);
   A->SetServerView(100,100,false,FText::FromString(TEXT("Knight")));
   TestEqual(TEXT("Respawn snapshot restores full"),A->HealthText->GetText().ToString(),FString(TEXT("100 / 100")));
  }
  auto* Component=NewObject<US1EntityHealthComponent>();
  TestTrue(TEXT("Screen-space independent of actor yaw"),Component->GetWidgetSpace()==EWidgetSpace::Screen);
  TestTrue(TEXT("No gameplay collision"),Component->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
  Component->bHideOnDeath=true;
  Component->SetServerView(100,100,false,FText::FromString(TEXT("Dusk")));
  TestTrue(TEXT("Living monster visible"),Component->IsVisible());
  Component->SetPresentationEnabled(false);
  Component->SetServerView(90,100,false,FText::FromString(TEXT("Dusk")));
  TestFalse(TEXT("Snapshot cannot override combat visibility gate"),Component->IsVisible());
  Component->SetPresentationEnabled(true);
  Component->SetServerView(0,100,true,FText::FromString(TEXT("Dusk")));
  TestFalse(TEXT("Dead monster hidden"),Component->IsVisible());
  Component->SetServerView(100,100,false,FText::FromString(TEXT("Dusk")));
  TestTrue(TEXT("New alive snapshot restores visibility"),Component->IsVisible());
  Component->SetServerView(0,0,false,FText::GetEmpty());
  TestFalse(TEXT("Unknown snapshot hidden"),Component->IsVisible());
  A->RemoveFromParent();B->RemoveFromParent();
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
