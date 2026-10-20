#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1MainHUDWidget.h"
#include "UI/S1EntityHealthComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Account/S1AccountSubsystem.h"
#include "UI/S1MinimapWidget.h"
#include "Protocol.pb.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1MainHUDTest,"S1.UI.MainHUD",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1MainHUDTest::RunTest(const FString&)
{
 auto* TestInstance=NewObject<UGameInstance>();
 auto* Account=NewObject<US1AccountSubsystem>(TestInstance);
 TestEqual(TEXT("No invented initial level"),Account->GetActiveCharacterLevel(),uint32(0));
 Protocol::S_ENTER_GAME Entry;Entry.set_success(true);Entry.mutable_player()->mutable_character_info()->set_level(27);
 Account->HandleEnterGamePacket(Entry);TestEqual(TEXT("Server entry level cached"),Account->GetActiveCharacterLevel(),uint32(27));
 Entry.set_success(false);Account->HandleEnterGamePacket(Entry);TestEqual(TEXT("Failed entry clears level"),Account->GetActiveCharacterLevel(),uint32(0));
 Entry.set_success(true);Account->HandleEnterGamePacket(Entry);Account->ResetAuthenticationState();
 TestEqual(TEXT("Session reset clears level"),Account->GetActiveCharacterLevel(),uint32(0));
 auto* Health=NewObject<US1EntityHealthComponent>();
 TestEqual(TEXT("No invented initial HP"),Health->GetServerMaxHP(),uint32(0));
 Health->SetServerView(63,100,false,FText::GetEmpty());
 TestEqual(TEXT("HUD reads existing server HP"),Health->GetServerHP(),uint32(63));
 TestFalse(TEXT("Alive snapshot"),Health->IsServerDead());
 Health->SetServerView(0,100,true,FText::GetEmpty());TestTrue(TEXT("Death snapshot"),Health->IsServerDead());
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("World"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1MainHUDWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1MainHUD.WBP_S1MainHUD_C"));
 auto* View=Class?CreateWidget<US1MainHUDWidget>(World,Class):nullptr;
 if(TestNotNull(TEXT("Saved MainHUD WBP"),View))
 {
  View->TakeWidget();View->SetCombatView(36,100,false,ES1KnightActionState::Ready,0);
  View->SetLevelView(27);TestTrue(TEXT("Actual server level display"),View->LevelText&&View->LevelText->GetText().ToString()==TEXT("Lv. 27"));
  View->SetLevelView(0);TestTrue(TEXT("Unknown level not invented"),View->LevelText&&View->LevelText->GetText().ToString()==TEXT("Lv. —"));
  TestTrue(TEXT("Health percent bound"),View->HealthBar&&FMath::IsNearlyEqual(View->HealthBar->GetPercent(),.36f));
  TestTrue(TEXT("Health number bound"),View->HealthText&&View->HealthText->GetText().ToString().Contains(TEXT("36 / 100")));
  View->SetCombatView(250,100,false,ES1KnightActionState::PrimaryCombo,3);
  TestTrue(TEXT("Percent clamped"),View->HealthBar&&View->HealthBar->GetPercent()==1.f);
  TestTrue(TEXT("Actual combo phase shown"),View->PrimaryText&&View->PrimaryText->GetText().ToString().Contains(TEXT("3 / 4")));
  View->SetCombatView(0,100,true,ES1KnightActionState::ChargePresentation,0);
  TestTrue(TEXT("Dead HP zero"),View->HealthBar&&View->HealthBar->GetPercent()==0.f);
  TestTrue(TEXT("Q animation indicator"),View->ChargeText&&View->ChargeText->GetText().ToString().Contains(TEXT("재생 중")));
  View->SetCombatView(0,0,false,ES1KnightActionState::Ready,0);
  TestTrue(TEXT("Unknown HP explicitly syncing"),View->HealthText&&View->HealthText->GetText().ToString().Contains(TEXT("동기화")));
  View->SetView({20001,0,0,0},7,TEXT("사용 가능"),true);
  auto* Count=Cast<UTextBlock>(View->GetWidgetFromName(TEXT("Count1")));
  auto* Icon=Cast<UImage>(View->GetWidgetFromName(TEXT("Icon1")));
  TestTrue(TEXT("Numeric quickslot stock"),Count&&Count->GetText().ToString()==TEXT("7"));
  TestTrue(TEXT("Existing potion icon"),Icon&&Icon->GetBrush().GetResourceObject()!=nullptr);
  TestTrue(TEXT("Decorative rounded HP brush"),View->HealthBar&&View->HealthBar->GetWidgetStyle().BackgroundImage.DrawAs==ESlateBrushDrawType::RoundedBox);
  float Baseline=-1;
  for(const TCHAR* Name:{TEXT("PrimaryFrame"),TEXT("ChargeFrame"),TEXT("SlotCell1"),TEXT("SlotCell2"),TEXT("SlotCell3"),TEXT("SlotCell4")})
  {
   auto* Cell=View->GetWidgetFromName(FName(Name));auto* Slot=Cell?Cast<UCanvasPanelSlot>(Cell->Slot):nullptr;
   if(TestNotNull(TEXT("Square row canvas slot"),Slot))
   {
    TestTrue(TEXT("Square 60px cell"),Slot->GetSize()==FVector2D(60,60));
    if(Baseline<0)Baseline=Slot->GetPosition().Y;
    TestEqual(TEXT("Common row baseline"),Slot->GetPosition().Y,double(Baseline));
   }
  }
  for(const TCHAR* Prefix:{TEXT("Primary"),TEXT("Charge")})
  {
   const FString P(Prefix);
   auto* SkillIcon=Cast<UImage>(View->GetWidgetFromName(FName(*(P+TEXT("Icon")))));
   auto* Shade=View->GetWidgetFromName(FName(*(P+TEXT("CooldownShade"))));
   auto* Timer=Cast<UTextBlock>(View->GetWidgetFromName(FName(*(P+TEXT("CooldownText")))));
   TestTrue(TEXT("Generated skill icon wired"),SkillIcon&&SkillIcon->GetVisibility()==ESlateVisibility::HitTestInvisible&&SkillIcon->GetBrush().GetResourceObject()!=nullptr);
   TestTrue(TEXT("No invented cooldown shade"),Shade&&Shade->GetVisibility()==ESlateVisibility::Collapsed);
   TestTrue(TEXT("No invented cooldown value"),Timer&&Timer->GetVisibility()==ESlateVisibility::Collapsed&&Timer->GetText().IsEmpty());
  }
  TestNotNull(TEXT("Inventory button"),View->InventoryButton.Get());TestNotNull(TEXT("Quest button"),View->QuestButton.Get());
  View->InventoryClicked();View->QuestClicked(); // No owning game instance: safe no-op, no request.
  TestNotNull(TEXT("Separate minimap child"),Cast<US1MinimapWidget>(View->GetWidgetFromName(TEXT("MinimapWidget"))));
  TestNotNull(TEXT("Separate currency child"),View->GetWidgetFromName(TEXT("CurrencyWidget")));
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1MinimapProjectionTest,"S1.UI.MinimapProjection",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1MinimapProjectionTest::RunTest(const FString&)
{
 const FVector2D Origin(0,0),Half(100,100);
 TestTrue(TEXT("Center"),US1MinimapWidget::ProjectPosition(FVector::ZeroVector,Origin,Half).Equals(FVector2D(.5,.5)));
 TestTrue(TEXT("North +X points up"),US1MinimapWidget::ProjectPosition(FVector(50,0,0),Origin,Half).Equals(FVector2D(.5,.25)));
 TestTrue(TEXT("East +Y points right"),US1MinimapWidget::ProjectPosition(FVector(0,50,0),Origin,Half).Equals(FVector2D(.75,.5)));
 TestTrue(TEXT("Marker clamped within map"),US1MinimapWidget::ProjectPosition(FVector(500,-500,0),Origin,Half).Equals(FVector2D(.04,.04)));
 TestTrue(TEXT("Bad extent cannot divide by zero"),US1MinimapWidget::ProjectPosition(FVector::ZeroVector,Origin,FVector2D::ZeroVector).Equals(FVector2D(.5,.5)));
 return true;
}
#endif
