#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "S1AuthWidget.h"
#include "S1CharacterSelectWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1AuthFocusTest,"S1.UI.AuthFocusTargets",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1AuthFocusTest::RunTest(const FString&)
{
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Auth=CreateWidget<US1AuthWidget>(World,US1AuthWidget::StaticClass());
 auto* Select=CreateWidget<US1CharacterSelectWidget>(World,US1CharacterSelectWidget::StaticClass());
 TestFalse(TEXT("No stale auth target before rebuild"),Auth->GetInitialFocusTarget().IsValid());
 TestFalse(TEXT("No stale selection target before rebuild"),Select->GetInitialFocusTarget().IsValid());
 Auth->TakeWidget();Select->TakeWidget();
 TestTrue(TEXT("Login uses editable account input"),Auth->GetInitialFocusTarget()==Auth->LoginAccountInput);
 TestTrue(TEXT("Actual login target accepts focus"),Auth->GetInitialFocusTarget()->SupportsKeyboardFocus());
 TestTrue(TEXT("Passwords stay masked"),Auth->LoginPasswordInput->IsPassword()&&Auth->SignUpPasswordInput->IsPassword()&&Auth->ConfirmPasswordInput->IsPassword());
 for(int i=0;i<3;++i)
 {
  const auto Open=Auth->HandleOpenSignUpClicked();
  TestTrue(TEXT("Signup targets visible signup input"),Auth->GetInitialFocusTarget()==Auth->SignUpAccountInput);
  TestTrue(TEXT("Signup click transfers same-user focus"),Open.GetUserFocusRecepient()==Auth->SignUpAccountInput);
  const auto Back=Auth->HandleBackToLoginClicked();
  TestTrue(TEXT("Back targets login input"),Back.GetUserFocusRecepient()==Auth->LoginAccountInput);
 }
 Auth->SetRequestInFlight(true);Auth->HandleOpenSignUpClicked();
 TestTrue(TEXT("Pending request still blocks screen switching"),Auth->GetInitialFocusTarget()==Auth->LoginAccountInput);
 Auth->SetRequestInFlight(false);
 Select->PlayButton->SetEnabled(false);
 TestTrue(TEXT("No character focuses create button"),Select->GetInitialFocusTarget()==Select->OpenCreateButton);
 Select->SelectedCharacterId=1;Select->PlayButton->SetEnabled(true);
 TestTrue(TEXT("Selected character focuses play"),Select->GetInitialFocusTarget()==Select->PlayButton);
 for(int i=0;i<3;++i)
 {
  auto Open=Select->HandleOpenCreateClicked();
  TestTrue(TEXT("Creation focuses name input"),Open.GetUserFocusRecepient()==Select->CharacterNameInput);
  TestTrue(TEXT("Name input accepts focus"),Select->GetInitialFocusTarget()->SupportsKeyboardFocus());
  auto Back=Select->HandleBackClicked();TestTrue(TEXT("Back returns to stable play button"),Back.GetUserFocusRecepient()==Select->PlayButton);
 }
 Select->SetRequestInFlight(true);
 TestFalse(TEXT("Disabled pending buttons not focused"),Select->GetInitialFocusTarget().IsValid());
 // Removed widgets have no viewport: async failures must not steal global focus.
 Auth->RestoreInputFocus();Select->RestoreInputFocus();
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
