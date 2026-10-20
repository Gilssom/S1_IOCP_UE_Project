#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Quest/S1QuestNpc.h"
#include "UI/S1NpcNameplateWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1NpcNameplateTest,"S1.Quest.NpcNameplate",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1NpcNameplateTest::RunTest(const FString&)
{
 TestFalse(TEXT("Initially hidden in hysteresis band"),US1NpcNameplateComponent::ShouldShowAtDistance(false,FMath::Square(1600.f),1500,1800));
 TestTrue(TEXT("Enter at fifteen meters"),US1NpcNameplateComponent::ShouldShowAtDistance(false,FMath::Square(1500.f),1500,1800));
 TestTrue(TEXT("Visible retained in band"),US1NpcNameplateComponent::ShouldShowAtDistance(true,FMath::Square(1700.f),1500,1800));
 TestFalse(TEXT("Hide beyond eighteen meters"),US1NpcNameplateComponent::ShouldShowAtDistance(true,FMath::Square(1801.f),1500,1800));
 TestTrue(TEXT("Invalid exit setting clamped to entry"),US1NpcNameplateComponent::ShouldShowAtDistance(true,FMath::Square(1400.f),1500,1000));
 auto* NpcClass=LoadClass<AS1QuestNpc>(nullptr,TEXT("/Game/S1/Quest/BP_S1QuestNpc.BP_S1QuestNpc_C"));
 if(!TestNotNull(TEXT("Authored NPC BP"),NpcClass))return false;
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("World"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Npc=World->SpawnActor<AS1QuestNpc>(NpcClass);
 if(TestNotNull(TEXT("NPC"),Npc)&&TestNotNull(TEXT("Independent nameplate component"),Npc->Nameplate.Get()))
 {
  auto* Plate=Npc->Nameplate.Get();
  TestTrue(TEXT("Screen-space name"),Plate->GetWidgetSpace()==EWidgetSpace::Screen);
  TestFalse(TEXT("Does not claim focus"),Plate->GetWindowFocusable());
  TestTrue(TEXT("No collision"),Plate->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
  TestTrue(TEXT("Name above prompt"),Plate->GetRelativeLocation().Z>210.f);
  auto* Viewer=World->SpawnActor<ACameraActor>();
  if(TestNotNull(TEXT("Local viewer stand-in"),Viewer))
  {
   TestEqual(TEXT("Show distance native default"),Plate->ShowDistance,1500.f);
   TestEqual(TEXT("Hide distance native default"),Plate->HideDistance,1800.f);
   Plate->UpdateForViewer(nullptr);TestFalse(TEXT("No local pawn hides plate"),Plate->IsVisible());
   Viewer->SetActorLocation(Npc->GetActorLocation()+FVector(1400,0,0));Plate->UpdateForViewer(Viewer);
   TestTrue(TEXT("Near pawn shows plate"),Plate->IsVisible());
   Viewer->SetActorLocation(Npc->GetActorLocation()+FVector(1700,0,0));Plate->UpdateForViewer(Viewer);
   TestTrue(TEXT("No flicker in band"),Plate->IsVisible());
   Viewer->SetActorLocation(Npc->GetActorLocation()+FVector(1900,0,0));Plate->UpdateForViewer(Viewer);
   TestFalse(TEXT("Far pawn hides plate"),Plate->IsVisible());
   Viewer->SetActorLocation(Npc->GetActorLocation()+FVector(1700,0,0));Plate->UpdateForViewer(Viewer);
   TestFalse(TEXT("Returning only to band remains hidden"),Plate->IsVisible());
   Viewer->SetActorLocation(Npc->GetActorLocation());Plate->UpdateForViewer(Viewer);
   TestTrue(TEXT("Reapproach shows again"),Plate->IsVisible());
   Plate->UpdateForViewer(nullptr);TestFalse(TEXT("Lost viewer clears visibility"),Plate->IsVisible());
  }
  Npc->DisplayName=FText::FromString(TEXT("이름 전달 시험"));Npc->RefreshNameplate();
  Plate->InitWidget();
  auto CheckName=[&](const TCHAR* Label)
  {
   auto* View=Cast<US1NpcNameplateWidget>(Plate->GetUserWidgetObject());
   if(!TestNotNull(TEXT("WBP inherits passive native name view"),View))return;
   View->TakeWidget();
   auto* Text=Cast<UTextBlock>(View->GetWidgetFromName(TEXT("NameText")));
   if(TestNotNull(TEXT("Designer NameText binding"),Text))TestEqual(Label,Text->GetText().ToString(),Npc->DisplayName.ToString());
   TestFalse(TEXT("Name view not focusable"),View->IsFocusable());
   TestTrue(TEXT("No mouse interception"),View->GetVisibility()==ESlateVisibility::HitTestInvisible);
  };
  CheckName(TEXT("Name survives late widget initialization"));
  Npc->DisplayName=FText::FromString(TEXT("이름 변경 시험"));Npc->RefreshNameplate();
  CheckName(TEXT("Explicit runtime refresh"));
  Plate->SetWidget(nullptr);Plate->InitWidget();
  CheckName(TEXT("Name survives widget recreation"));
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
