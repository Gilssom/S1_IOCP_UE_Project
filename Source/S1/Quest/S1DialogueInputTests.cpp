#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/S1DialogueWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1DialogueInputTest,"S1.Quest.DialogueGKey",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1DialogueInputTest::RunTest(const FString&)
{
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("World"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Class=LoadClass<US1DialogueWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1Dialogue.WBP_S1Dialogue_C"));
 auto* View=Class?CreateWidget<US1DialogueWidget>(World,Class):nullptr;
 if(TestNotNull(TEXT("Authored dialogue WBP"),View))
 {
  View->TakeWidget();int Advances=0,Accepts=0,Closes=0;
  const FGeometry Geometry;
  auto Key=[](FKey K,bool bRepeat){return FKeyEvent(K,FModifierKeysState(),0,bRepeat,0,0);};
  View->OnAdvanceLine.BindLambda([&]{++Advances;});
  View->OnPrimary.BindLambda([&]{++Accepts;});View->OnClose.BindLambda([&]{++Closes;});
  TestTrue(TEXT("G tunnel consumed"),View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::G,false)).IsEventHandled());
  TestEqual(TEXT("One press, one next"),Advances,1);TestEqual(TEXT("No acceptance callback"),Accepts,0);
  View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::G,true));TestEqual(TEXT("Hold repeat ignored"),Advances,1);
  View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::G,false));TestEqual(TEXT("New press advances again"),Advances,2);
  View->OnAdvanceLine.Unbind(); // Offer/End node policy from subsystem.
  TestTrue(TEXT("Offer G consumed without action"),View->NativeOnPreviewKeyDown(Geometry,Key(EKeys::G,false)).IsEventHandled());
  TestEqual(TEXT("Offer never accepts through G"),Accepts,0);TestEqual(TEXT("Offer never advances through G"),Advances,2);
  View->NativeOnKeyDown(Geometry,Key(EKeys::Escape,false));TestEqual(TEXT("Esc close preserved"),Closes,1);
  View->NativeDestruct();TestFalse(TEXT("Destruct clears advance callback"),View->OnAdvanceLine.IsBound());
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
