#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Quest/S1DialogueSubsystem.h"
#include "Quest/S1QuestNpc.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerController.h"
#include "Game/S1MyPlayer.h"
#include "Game/S1Player.h"
#include "UI/S1EntityHealthComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "UI/S1DialogueWidget.h"
#include "UI/S1NpcNameplateWidget.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1DialogueTest,"S1.Quest.DialogueStageA",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1DialogueTest::RunTest(const FString&)
{
 TestTrue(TEXT("Near NPC wins"),US1DialogueSubsystem::PreferNpc(100.f*100.f,140.f*140.f,false));
 TestFalse(TEXT("Near loot wins"),US1DialogueSubsystem::PreferNpc(180.f*180.f,100.f*100.f,true));
 TestTrue(TEXT("NPC hysteresis prevents flicker"),US1DialogueSubsystem::PreferNpc(120.f*120.f,110.f*110.f,true));
 TestFalse(TEXT("Loot hysteresis prevents flicker"),US1DialogueSubsystem::PreferNpc(100.f*100.f,110.f*110.f,false));
 const FVector Focus=US1DialogueSubsystem::TwoShotFocus(FVector(0,0,0),FVector(200,0,0),100);
 TestTrue(TEXT("Two-shot uses feet, not capsule-center duplicate offset"),Focus.Equals(FVector(100,0,100)));
 for(const FVector& Side:{FVector(200,0,0),FVector(0,200,0),FVector(-200,0,0),FVector(0,-200,0)})
  for(bool bMirror:{false,true})
   TestTrue(TEXT("Every approach keeps camera on player's side of NPC"),
    FVector::DotProduct(Side.GetSafeNormal(),US1DialogueSubsystem::PlayerSideCameraOffset(Side,FVector::ZeroVector,FVector(-320,240,160),bMirror).GetSafeNormal())>0.f);
 auto* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("World"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Dialogue=World->GetSubsystem<US1DialogueSubsystem>();
 if(!TestNotNull(TEXT("Registered dialogue subsystem"),Dialogue))return false;
 auto* Camera=World->SpawnActor<ACameraActor>();
 US1DialogueSubsystem::ConfigureDialogueCamera(Camera,60.f);
 TestFalse(TEXT("No opaque camera letterbox"),Camera->GetCameraComponent()->bConstrainAspectRatio);
 TestEqual(TEXT("Dialogue FOV preserved"),Camera->GetCameraComponent()->FieldOfView,60.f);
 TestEqual(TEXT("Edge shade defaults to half transparent"),GetDefault<US1DialogueWidget>()->EdgeShadeOpacity,.5f);
 TestTrue(TEXT("Soft edge occupies only eight percent each"),FMath::IsNearlyEqual(GetDefault<US1DialogueWidget>()->EdgeShadeHeightFraction,.08f));
 auto* DialogueClass=LoadClass<US1DialogueWidget>(nullptr,TEXT("/Game/S1/UI/WBP_S1Dialogue.WBP_S1Dialogue_C"));
 if(TestNotNull(TEXT("Authored dialogue WBP"),DialogueClass))
 {
  auto* View=CreateWidget<US1DialogueWidget>(World,DialogueClass);
  if(TestNotNull(TEXT("Dialogue view"),View))
  {
   const auto Slate=View->TakeWidget();
   TestTrue(TEXT("Gradient wrapper builds with authored WBP tree"),Slate->GetChildren()->Num()>0);
   for(const FName Name:{TEXT("NpcNameText"),TEXT("DialogueText"),TEXT("QuestTitleText"),TEXT("QuestDetailText"),TEXT("AcceptButton"),TEXT("DeclineButton")})
    TestNotNull(TEXT("Authored dialogue controls survive Slate wrapper"),View->GetWidgetFromName(Name));
   TestEqual(TEXT("Existing WBP inherits half-alpha native default"),View->EdgeShadeOpacity,.5f);
  }
 }
 TestFalse(TEXT("Closed initially"),Dialogue->IsModal());
 Dialogue->Phase=US1DialogueSubsystem::EPhase::FadeToDialogue;
 TestTrue(TEXT("All fade phases own modal input"),Dialogue->IsModal());
 Dialogue->AbortDialogue();Dialogue->AbortDialogue();
 TestFalse(TEXT("Repeated early abort is safe and releases modal"),Dialogue->IsModal());
 auto* PC=World->SpawnActor<APlayerController>();
 if(PC)
 {
  PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true); // unrelated owner
  PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true); // dialogue lease
  Dialogue->OwnerPC=PC;Dialogue->Phase=US1DialogueSubsystem::EPhase::Active;
  Dialogue->AbortDialogue();Dialogue->AbortDialogue();
  TestTrue(TEXT("Abort preserves another input owner"),PC->IsMoveInputIgnored()&&PC->IsLookInputIgnored());
  PC->SetIgnoreMoveInput(false);PC->SetIgnoreLookInput(false);
  TestFalse(TEXT("Dialogue lease released exactly once"),PC->IsMoveInputIgnored()||PC->IsLookInputIgnored());
 }
 auto* Npc=World->SpawnActor<AS1QuestNpc>();
 if(TestNotNull(TEXT("NPC"),Npc))
 {
  TestNotNull(TEXT("Swappable Manny mesh"),Npc->NpcMesh.Get());
  TestNotNull(TEXT("Swappable idle"),Npc->IdleAnimation.Get());
  if(Npc->NpcMesh&&Npc->IdleAnimation)TestTrue(TEXT("Manny idle shares skeleton"),Npc->NpcMesh->GetSkeleton()==Npc->IdleAnimation->GetSkeleton());
  TestFalse(TEXT("No RPC replication authority"),Npc->GetIsReplicated());
  TestTrue(TEXT("Exit radius tolerates entry boundary"),Npc->ExitRadius>=Npc->InteractionRadius);
  TestTrue(TEXT("Stage A explicitly says acceptance unavailable"),Npc->QuestPreview.ToString().Contains(TEXT("지원하지 않습니다")));
  auto* Local=World->SpawnActor<AS1MyPlayer>();
  auto* Remote=World->SpawnActor<AS1Player>();
  auto* PreHidden=World->SpawnActor<AS1Player>();
  auto* OtherPC=World->SpawnActor<APlayerController>();
  if(Local&&Remote&&PreHidden&&OtherPC&&PC)
  {
   Local->SetActorLocation(Npc->GetActorLocation()+FVector(0,200,90));
   const FTransform PlayerBefore=Local->GetActorTransform();
   const FTransform NpcBefore=Npc->GetActorTransform();
   const FRotator MeshBefore=Npc->Visual->GetRelativeRotation();
   PC->HiddenActors.Add(PreHidden);
   Dialogue->OwnerPC=PC;Dialogue->OwnerPawn=Local;Dialogue->ActiveNpc=Npc;
   Dialogue->ApplyPrivatePresentation();
   TestTrue(TEXT("Only NPC visual turns, not actor/server placement"),Npc->GetActorTransform().Equals(NpcBefore));
   TestFalse(TEXT("NPC mesh faces player approaching from side"),Npc->Visual->GetRelativeRotation().Equals(MeshBefore));
   TestTrue(TEXT("Player not teleported or rotated"),Local->GetActorTransform().Equals(PlayerBefore));
   TestTrue(TEXT("Remote hidden in this controller"),PC->HiddenActors.Contains(Remote));
   TestFalse(TEXT("Local player remains visible"),PC->HiddenActors.Contains(Local));
   TestTrue(TEXT("Another controller render mask untouched"),OtherPC->HiddenActors.IsEmpty());
   auto* Bar=Remote->FindComponentByClass<US1EntityHealthComponent>();
   if(Bar)
   {
    Bar->SetServerView(80,100,false,FText::GetEmpty());Bar->SetPresentationEnabled(true);
    TestFalse(TEXT("HP refresh cannot undo dialogue mask"),Bar->IsVisible());
   }
   auto* LateWeapon=World->SpawnActor<ACameraActor>();
   LateWeapon->AttachToActor(Remote,FAttachmentTransformRules::KeepWorldTransform);
   Dialogue->ApplyPrivatePresentation();
   TestTrue(TEXT("Late attached actor also hidden"),PC->HiddenActors.Contains(LateWeapon));
   Dialogue->RestorePrivatePresentation();Dialogue->RestorePrivatePresentation();
   TestTrue(TEXT("NPC original mesh rotation restored"),Npc->Visual->GetRelativeRotation().Equals(MeshBefore));
   TestFalse(TEXT("Remote restored"),PC->HiddenActors.Contains(Remote));
   TestFalse(TEXT("Attachment restored"),PC->HiddenActors.Contains(LateWeapon));
   TestTrue(TEXT("Pre-existing hidden actor preserved"),PC->HiddenActors.Contains(PreHidden));
   if(Bar)TestTrue(TEXT("HP restored using latest policy, not stale visibility"),Bar->IsVisible());
   auto* LocalBar=Local->FindComponentByClass<US1EntityHealthComponent>();
   if(LocalBar)
   {
    LocalBar->SetPresentationEnabled(true);LocalBar->SetServerView(70,100,false,FText::GetEmpty());
    Dialogue->Phase=US1DialogueSubsystem::EPhase::Active;
    LocalBar->TickComponent(0,LEVELTICK_All,nullptr);
    TestFalse(TEXT("Own HP/name hidden during dialogue"),LocalBar->IsVisible());
    LocalBar->SetServerView(90,100,false,FText::GetEmpty());
    TestFalse(TEXT("HP updates cannot unhide own label"),LocalBar->IsVisible());
    Npc->Nameplate->UpdateForViewer(Local);
    TestFalse(TEXT("NPC name hidden during dialogue"),Npc->Nameplate->IsVisible());
    Dialogue->Phase=US1DialogueSubsystem::EPhase::Closed;
    LocalBar->TickComponent(0,LEVELTICK_All,nullptr);
    TestTrue(TEXT("Own HP/name restored without new HP packet"),LocalBar->IsVisible());
    Npc->Nameplate->UpdateForViewer(Local);
    TestTrue(TEXT("Nearby NPC name restored after dialogue"),Npc->Nameplate->IsVisible());
   }
  }
 }
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return !HasAnyErrors();
}
#endif
