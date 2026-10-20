#include "Quest/S1DialogueSubsystem.h"
#include "Quest/S1QuestNpc.h"
#include "Quest/S1QuestSubsystem.h"
#include "UI/S1DialogueWidget.h"
#include "Game/S1MyPlayer.h"
#include "Game/S1Player.h"
#include "UI/S1EntityHealthComponent.h"
#include "Game/S1DuskMinion.h"
#include "Combat/S1KnightCombatComponent.h"
#include "Loot/S1InventorySubsystem.h"
#include "Level/S1LevelFlowSubsystem.h"
#include "Network/S1NetworkSubsystem.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"

bool US1DialogueSubsystem::DoesSupportWorldType(EWorldType::Type Type) const
{return Type==EWorldType::Game||Type==EWorldType::PIE;}
TStatId US1DialogueSubsystem::GetStatId() const {RETURN_QUICK_DECLARE_CYCLE_STAT(US1DialogueSubsystem,STATGROUP_Tickables);}
AS1MyPlayer* US1DialogueSubsystem::LocalPlayer() const
{auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;return PC&&PC->IsLocalController()?Cast<AS1MyPlayer>(PC->GetPawn()):nullptr;}
bool US1DialogueSubsystem::PreferNpc(float NpcDistanceSquared,float LootDistanceSquared,bool bRetainNpc)
{return FMath::Sqrt(FMath::Max(0.f,NpcDistanceSquared)) < FMath::Sqrt(FMath::Max(0.f,LootDistanceSquared))+(bRetainNpc?20.f:-20.f);}
FVector US1DialogueSubsystem::TwoShotFocus(const FVector& PlayerFeet,const FVector& NpcFeet,float Height)
{return (PlayerFeet+NpcFeet)*.5f+FVector(0,0,Height);}
FVector US1DialogueSubsystem::PlayerSideCameraOffset(const FVector& PlayerFeet,const FVector& NpcFeet,const FVector& Offset,bool bMirrorShoulder)
{
 FVector TowardPlayer=(PlayerFeet-NpcFeet).GetSafeNormal2D();
 if(TowardPlayer.IsNearlyZero())TowardPlayer=FVector::ForwardVector;
 const FVector Shoulder=FVector::CrossProduct(FVector::UpVector,TowardPlayer);
 return TowardPlayer*FMath::Max(60.,FMath::Abs(Offset.X))+
  Shoulder*Offset.Y*(bMirrorShoulder?-1.:1.)+FVector(0,0,Offset.Z);
}
void US1DialogueSubsystem::ApplyPrivatePresentation()
{
 auto* PC=OwnerPC.Get();auto* Player=OwnerPawn.Get();auto* Npc=ActiveNpc.Get();
 if(!PC||!Player||!Npc)return;
 if(!bPrivatePresentation)
 {
  bPrivatePresentation=true;
  if(auto* Mesh=Npc->Visual.Get())
  {
   TurnedNpcMesh=Mesh;SavedNpcMeshRotation=Mesh->GetRelativeRotation();
   const FVector Direction=(Player->GetActorLocation()-Npc->GetActorLocation()).GetSafeNormal2D();
   if(!Direction.IsNearlyZero())Mesh->SetWorldRotation(FRotator(0,Direction.Rotation().Yaw+Npc->MeshYaw,0));
  }
 }
 // Each PIE/client has its own World. Only this local controller's render mask changes.
 // Re-evaluate every tick so arrivals/late weapon attachments cannot leak into the shot.
 for(TActorIterator<AS1Player> It(GetWorld());It;++It)
 {
  if(*It==Player)continue;
  TArray<AActor*> Actors;It->GetAttachedActors(Actors,true,true);Actors.Add(*It);
  for(AActor* Actor:Actors)
  {
   if(!IsValid(Actor)||Actor==Player||Actor==Npc)continue;
   if(!PC->HiddenActors.Contains(Actor))
   {PC->HiddenActors.Add(Actor);AddedHiddenActors.AddUnique(Actor);}
   TInlineComponentArray<US1EntityHealthComponent*> Health;Actor->GetComponents(Health);
   for(auto* Bar:Health)if(!Bar->IsDialogueSuppressed())
   {Bar->SetDialogueSuppressed(true);SuppressedHealth.AddUnique(Bar);}
  }
 }
}
void US1DialogueSubsystem::RestorePrivatePresentation()
{
 if(auto* Mesh=TurnedNpcMesh.Get())Mesh->SetRelativeRotation(SavedNpcMeshRotation);
 TurnedNpcMesh.Reset();bPrivatePresentation=false;
 if(auto* PC=OwnerPC.Get())
  PC->HiddenActors.RemoveAll([this](const TObjectPtr<AActor>& Actor)
  {return AddedHiddenActors.Contains(TWeakObjectPtr<AActor>(Actor.Get()));});
 AddedHiddenActors.Reset();
 for(auto& Weak:SuppressedHealth)if(auto* Bar=Weak.Get())Bar->SetDialogueSuppressed(false);
 SuppressedHealth.Reset();
}
bool US1DialogueSubsystem::HasCombatThreat() const
{
 if(FPlatformTime::Seconds()-LastDamageAt<3.)return true;
 for(TActorIterator<AS1DuskMinion> It(GetWorld());It;++It)if(It->IsThreateningLocalPlayer())return true;
 return false;
}
bool US1DialogueSubsystem::IsAvailable(AS1MyPlayer* Player) const
{
 if(!Player||!Player->IsLocallyControlled()||!GetWorld()->GetGameInstance())return false;
 const auto* GI=GetWorld()->GetGameInstance();
 const auto* Network=GI->GetSubsystem<US1NetworkSubsystem>();
 const auto* Flow=GI->GetSubsystem<US1LevelFlowSubsystem>();
 const auto* Inventory=GI->GetSubsystem<US1InventorySubsystem>();
 const auto* PC=Cast<APlayerController>(Player->GetController());
 return Network&&Network->IsConnected()&&Flow&&Flow->IsMainLevelReady()&&PC&&!PC->IsPaused()&&
  !PC->bShowMouseCursor&&Inventory&&!Inventory->IsInventoryOpen()&&
  Player->GetCharacterMovement()->IsMovingOnGround()&&
  Player->GetKnightCombatComponent()->GetActionState()==ES1KnightActionState::Ready&&!HasCombatThreat();
}
bool US1DialogueSubsystem::CanTalk(AS1MyPlayer* Player,AS1QuestNpc* Npc,float Radius) const
{
 if(!IsValid(Player)||!IsValid(Npc)||Npc->NpcId<=0)return false;
 const FVector Feet=Player->GetActorLocation()-FVector(0,0,Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
 if(FVector::DistSquaredXY(Feet,Npc->GetActorLocation())>FMath::Square(Radius)||
  FMath::Abs(Feet.Z-Npc->GetActorLocation().Z)>100.f)return false;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(S1NpcSight),false,Player);Query.AddIgnoredActor(Npc);
 return !GetWorld()->LineTraceTestByChannel(Feet+FVector(0,0,100),Npc->GetActorLocation()+FVector(0,0,100),ECC_Visibility,Query);
}
void US1DialogueSubsystem::RefreshCandidates()
{
 AS1MyPlayer* Player=LocalPlayer();
 auto* Inventory=GetWorld()->GetGameInstance()?GetWorld()->GetGameInstance()->GetSubsystem<US1InventorySubsystem>():nullptr;
 const bool bCanNpc=!IsModal()&&IsAvailable(Player)&&Inventory&&!Inventory->HasPendingPickup();
 AS1QuestNpc* Best=nullptr;float BestDistance=MAX_flt;
 for(TActorIterator<AS1QuestNpc> It(GetWorld());It;++It)
 {
  It->ShowPrompt(false);
  if(!bCanNpc||!CanTalk(Player,*It,It->InteractionRadius))continue;
  const float Distance=FVector::DistSquaredXY(Player->GetActorLocation(),It->GetActorLocation());
  const float Score=FMath::Sqrt(Distance)-(CandidateNpc.Get()==*It?20.f:0.f);
  if(!Best||Score<BestDistance||(Score==BestDistance&&It->NpcId<Best->NpcId)){Best=*It;BestDistance=Score;}
 }
 float LootDistance=MAX_flt;
 const FString Loot=Inventory&&!IsModal()?Inventory->GetPickupCandidate(LootDistance):FString();
 if(Best&&!Loot.IsEmpty()&&!PreferNpc(FVector::DistSquaredXY(Player->GetActorLocation(),Best->GetActorLocation()),LootDistance,CandidateNpc==Best))Best=nullptr;
 CandidateNpc=Best;SelectedLoot=Best?FString():Loot;
 if(Best)Best->ShowPrompt(true);
 if(Inventory)Inventory->UpdateInteractionHint(IsModal()||Best?FString():SelectedLoot,IsModal());
}
bool US1DialogueSubsystem::ShouldShowLoot(const FString& Key) const
{return !IsModal()&&!CandidateNpc.IsValid()&&SelectedLoot==Key;}
void US1DialogueSubsystem::Interact()
{
 if(IsModal())return;
 // Use the displayed selection; validate it, never execute both handlers.
 if(auto* Npc=CandidateNpc.Get())
 {if(IsAvailable(LocalPlayer())&&CanTalk(LocalPlayer(),Npc,Npc->InteractionRadius))BeginDialogue(Npc);return;}
 if(auto* Inventory=GetWorld()->GetGameInstance()->GetSubsystem<US1InventorySubsystem>())
  if(Inventory->HasPendingPickup()||!SelectedLoot.IsEmpty())Inventory->RequestPickup(SelectedLoot);
}
void US1DialogueSubsystem::BeginDialogue(AS1QuestNpc* Npc)
{
 ++DialogueRun;
 auto* Player=LocalPlayer();auto* PC=Player?Cast<APlayerController>(Player->GetController()):nullptr;
 if(IsModal()||!PC||!IsAvailable(Player)||!CanTalk(Player,Npc,Npc->InteractionRadius)||!Npc->DialogueWidgetClass)return;
 OwnerPC=PC;OwnerPawn=Player;ActiveNpc=Npc;SavedViewTarget=PC->GetViewTarget();SavedControlRotation=PC->GetControlRotation();bSavedCursor=PC->bShowMouseCursor;
 Phase=EPhase::FadeToDialogue;PhaseTime=0;FadeDuration=FMath::Clamp(Npc->FadeSeconds,.05f,1.f);
 Player->StopForDialogue();
 PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true);
 PC->bShowMouseCursor=true;PC->SetInputMode(FInputModeUIOnly());
 if(PC->PlayerCameraManager)PC->PlayerCameraManager->StartCameraFade(0,1,FadeDuration,FLinearColor::Black,false,true);
 RefreshCandidates();
 UE_LOG(LogTemp,Log,TEXT("Dialogue begin. Npc[%d] Local-only stage A; no quest request."),Npc->NpcId);
}
void US1DialogueSubsystem::ConfigureDialogueCamera(ACameraActor* Camera,float FOV)
{
 if(!Camera)return;
 Camera->GetCameraComponent()->SetFieldOfView(FOV);
 // ACameraActor defaults true: its black letterbox cannot be alpha blended.
 Camera->GetCameraComponent()->bConstrainAspectRatio=false;
}
void US1DialogueSubsystem::RevealDialogue()
{
 auto* PC=OwnerPC.Get();auto* Player=OwnerPawn.Get();auto* Npc=ActiveNpc.Get();
 if(!PC||!Player||!Npc){AbortDialogue();return;}
 ApplyPrivatePresentation();
 const FVector Feet=Player->GetActorLocation()-FVector(0,0,Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
 const FVector Focus=TwoShotFocus(Feet,Npc->GetActorLocation(),Npc->FocusHeight);
 FCollisionQueryParams Query(SCENE_QUERY_STAT(S1DialogueCamera),false,Player);Query.AddIgnoredActor(Npc);
 for(const auto& Hidden:PC->HiddenActors)if(IsValid(Hidden.Get()))Query.AddIgnoredActor(Hidden.Get());
 int32 Width=0,Height=0;PC->GetViewportSize(Width,Height);
 const float Aspect=Height>0?static_cast<float>(Width)/Height:16.f/9.f;
 const float HalfAngle=FMath::Min(FMath::DegreesToRadians(Npc->DialogueFOV*.5f),FMath::Atan(FMath::Tan(FMath::DegreesToRadians(Npc->DialogueFOV*.5f))/FMath::Max(.5f,Aspect)));
 const float RequiredDistance=(FVector::Distance(Feet,Npc->GetActorLocation())*.5f+115.f)/FMath::Tan(HalfAngle*.85f);
 bool bFound=false;
 for(int32 Attempt=0;Attempt<3;++Attempt)
 {
  FVector AuthoredOffset=Npc->CameraOffset;if(Attempt==2)AuthoredOffset.Y=0;
  FVector Offset=PlayerSideCameraOffset(Feet,Npc->GetActorLocation(),AuthoredOffset,Attempt==1);
  if(Offset.IsNearlyZero())Offset=FVector(-1,1,.3f)*400;
  Offset=Offset.GetSafeNormal()*FMath::Max(RequiredDistance,Offset.Size());
  const FVector Location=Focus+Offset;
  FHitResult Hit;
  if(GetWorld()->SweepSingleByChannel(Hit,Focus,Location,FQuat::Identity,ECC_Camera,FCollisionShape::MakeSphere(12),Query)||
   GetWorld()->OverlapBlockingTestByChannel(Location,FQuat::Identity,ECC_Camera,FCollisionShape::MakeSphere(12),Query)||
   GetWorld()->LineTraceTestByChannel(Location,Feet+FVector(0,0,100),ECC_Visibility,Query)||
   GetWorld()->LineTraceTestByChannel(Location,Npc->GetActorLocation()+FVector(0,0,100),ECC_Visibility,Query))continue;
  DialogueCamera=GetWorld()->SpawnActor<ACameraActor>(Location,(Focus-Location).Rotation());
  if(DialogueCamera){ConfigureDialogueCamera(DialogueCamera,Npc->DialogueFOV);PC->SetViewTarget(DialogueCamera);bCameraChanged=true;bFound=true;}break;
 }
 DialogueWidget=CreateWidget<US1DialogueWidget>(PC,Npc->DialogueWidgetClass);
 if(!DialogueWidget){AbortDialogue();return;}
 DialogueWidget->OnClose.BindUObject(this,&US1DialogueSubsystem::CloseDialogue);
 DialogueWidget->AddToViewport(100);
 DialogueWidget->SetDialogue(Npc->DisplayName,Npc->Dialogue,Npc->QuestTitle,Npc->QuestPreview);
 DataSession.Reset();
 if(Npc->QuestDefinitions||Npc->DialogueNodes)
 {
  FString Error;
  int32 StartNode=Npc->StartDialogueNodeId;
  const auto* Runtime=GetWorld()->GetGameInstance()->GetSubsystem<US1QuestSubsystem>();
  if(Runtime&&Runtime->GetState().quest_id()==static_cast<uint32>(Npc->DialogueQuestId)&&Runtime->GetState().return_npc_id()==static_cast<uint32>(Npc->NpcId))
  {
   const auto& State=Runtime->GetState();
   const int32 Candidate=State.rewarded()?Npc->RewardedDialogueNodeId:
    State.status()==Protocol::QUEST_READY_TO_TURN_IN?Npc->TurnInDialogueNodeId:State.status()==Protocol::QUEST_ACTIVE?Npc->InProgressDialogueNodeId:0;
   if(Candidate>0)StartNode=Candidate;
  }
  if(!DataSession.Start(Npc->QuestDefinitions,Npc->DialogueNodes,Npc->NpcId,StartNode,Error))
  {UE_LOG(LogTemp,Warning,TEXT("Dialogue data rejected. Npc[%d]: %s"),Npc->NpcId,*Error);AbortDialogue();return;}
  DialogueWidget->OnPrimary.BindUObject(this,&US1DialogueSubsystem::PrimaryDialogueAction);
  DialogueWidget->OnSecondary.BindUObject(this,&US1DialogueSubsystem::SecondaryDialogueAction);
  RefreshDialogueNode();
 }
 FInputModeUIOnly Input;Input.SetWidgetToFocus(DialogueWidget->TakeWidget());PC->SetInputMode(Input);
 if(PC->PlayerCameraManager)PC->PlayerCameraManager->StartCameraFade(1,0,FadeDuration,FLinearColor::Black,false,false);
 Phase=EPhase::RevealDialogue;PhaseTime=0;
 UE_LOG(LogTemp,Log,TEXT("Dialogue camera. Npc[%d] TwoShot[%d] (0=safe gameplay-camera fallback)"),Npc->NpcId,bFound);
}
void US1DialogueSubsystem::CloseDialogue()
{
 if(!IsModal()||Phase==EPhase::FadeToGame||Phase==EPhase::RevealGame)return;
 if(DialogueWidget)DialogueWidget->SetVisibility(ESlateVisibility::Collapsed);
 if(auto* PC=OwnerPC.Get();PC&&PC->PlayerCameraManager)PC->PlayerCameraManager->StartCameraFade(0,1,FadeDuration,FLinearColor::Black,false,true);
 Phase=EPhase::FadeToGame;PhaseTime=0;
}
void US1DialogueSubsystem::RefreshDialogueNode()
{
 auto* Npc=ActiveNpc.Get();if(!Npc||!DialogueWidget)return;
 const auto* Runtime=GetWorld()->GetGameInstance()->GetSubsystem<US1QuestSubsystem>();
 const auto* Node=FS1QuestData::FindNode(Npc->DialogueNodes,DataSession.CurrentNodeId);
 if(!Node){AbortDialogue();return;}
 const bool bOwnQuest=Runtime&&Runtime->GetState().quest_id()==static_cast<uint32>(Npc->DialogueQuestId)&&Runtime->GetState().return_npc_id()==static_cast<uint32>(Npc->NpcId);
 if(bOwnQuest&&(Node->Kind==ES1DialogueNodeKind::QuestOffer||DataSession.CurrentNodeId==Npc->TurnInDialogueNodeId||DataSession.CurrentNodeId==Npc->InProgressDialogueNodeId))
 {
  const auto& State=Runtime->GetState();
  const int32 Candidate=State.rewarded()?Npc->RewardedDialogueNodeId:
   State.status()==Protocol::QUEST_READY_TO_TURN_IN?Npc->TurnInDialogueNodeId:State.status()==Protocol::QUEST_ACTIVE?Npc->InProgressDialogueNodeId:0;
  if(Candidate>0&&Candidate!=DataSession.CurrentNodeId)
  {
   const auto* Next=FS1QuestData::FindNode(Npc->DialogueNodes,Candidate);
   if(!Next||Next->NpcId!=Npc->NpcId){AbortDialogue();return;}
   DataSession.CurrentNodeId=Candidate;Node=Next;
  }
 }
 const bool bOffer=Node->Kind==ES1DialogueNodeKind::QuestOffer;
 const bool bTurnIn=bOwnQuest&&DataSession.CurrentNodeId==Npc->TurnInDialogueNodeId&&Node->Kind==ES1DialogueNodeKind::End;
 const bool bQuestInfo=bTurnIn||(bOwnQuest&&(DataSession.CurrentNodeId==Npc->InProgressDialogueNodeId||DataSession.CurrentNodeId==Npc->RewardedDialogueNodeId));
 DialogueWidget->OnAdvanceLine.Unbind();
 if(Node->Kind==ES1DialogueNodeKind::Line)DialogueWidget->OnAdvanceLine.BindUObject(this,&US1DialogueSubsystem::PrimaryDialogueAction);
 const auto* Quest=bOffer?FS1QuestData::FindQuest(Npc->QuestDefinitions,Node->QuestId):nullptr;
 FText Title,Detail;
 if(Quest)
 {
  Title=Quest->Title;
  Detail=FText::FromString(FString::Printf(TEXT("%s\n\n목표: Dusk %d마리\n목표 달성 후 지정 NPC에게 완료 보고하세요.\n%s"),*Quest->Description.ToString(),Quest->RequiredKills,Runtime?*Runtime->GetStatusText():TEXT("서버 상태 미확인")));
 }
 if(bQuestInfo)
 {
  const auto& State=Runtime->GetState();Title=FText::FromString(UTF8_TO_TCHAR(State.title().c_str()));
  Detail=FText::FromString(FString::Printf(TEXT("Dusk %u / %u\n%s\n%s"),State.progress(),State.required(),
   State.rewarded()?TEXT("보상 수령 완료"):Runtime->SupportsRewards()?TEXT("완료 보고 후 서버가 지급합니다"):TEXT("서버 보상 기능 미준비"),*Runtime->GetStatusText()));
 }
 DialogueWidget->SetDialogue(Node->Speaker,Node->Body,Title,Detail);
 DialogueWidget->SetReward(Quest?Quest->RewardItemId:bQuestInfo?Runtime->GetState().reward_item_id():0,
  Quest?Quest->RewardCount:bQuestInfo?Runtime->GetState().reward_count():0);
 const FString Primary=bTurnIn?TEXT("완료 보고 / 보상 재확인"):bOffer?TEXT("수락 / 결과 재확인"):(Node->Kind==ES1DialogueNodeKind::Line?TEXT("다음 [G]"):TEXT(""));
 const bool bEnabled=bTurnIn?Runtime->CanTurnIn(Npc->NpcId):!bOffer||(Quest&&Quest->bEnabled&&Runtime&&Runtime->CanAccept());
 DialogueWidget->SetNavigation(FText::FromString(Primary),FText::FromString(bOffer?TEXT("거절"):TEXT("닫기")),bEnabled,bOffer||bQuestInfo);
}
void US1DialogueSubsystem::PrimaryDialogueAction()
{
 if(Phase!=EPhase::Active)return;
 auto* Npc=ActiveNpc.Get();if(!Npc)return;
 const auto* Node=FS1QuestData::FindNode(Npc->DialogueNodes,DataSession.CurrentNodeId);if(!Node)return;
 if(Npc->TurnInDialogueNodeId>0&&DataSession.CurrentNodeId==Npc->TurnInDialogueNodeId&&Node->Kind==ES1DialogueNodeKind::End)
 {
  auto* Runtime=GetWorld()->GetGameInstance()->GetSubsystem<US1QuestSubsystem>();if(!Runtime)return;
  const TWeakObjectPtr<US1DialogueSubsystem> Weak=this;const TWeakObjectPtr<AS1QuestNpc> OriginalNpc=Npc;const uint64 Run=DialogueRun;
  Runtime->RequestTurnIn(Npc->NpcId,[Weak,OriginalNpc,Run]
  {
   auto* Self=Weak.Get();auto* N=OriginalNpc.Get();
   if(!Self||!N||Self->Phase!=EPhase::Active||Self->DialogueRun!=Run||Self->ActiveNpc!=OriginalNpc||Self->DataSession.CurrentNodeId!=N->TurnInDialogueNodeId)return;
   const auto* End=FS1QuestData::FindNode(N->DialogueNodes,N->RewardedDialogueNodeId);
   if(!End||End->NpcId!=N->NpcId||End->Kind!=ES1DialogueNodeKind::End)return;
   Self->DataSession.CurrentNodeId=N->RewardedDialogueNodeId;Self->RefreshDialogueNode();
  });
  RefreshDialogueNode();return;
 }
 if(Node->Kind==ES1DialogueNodeKind::QuestOffer)
 {
  const auto* Q=FS1QuestData::FindQuest(Npc->QuestDefinitions,Node->QuestId);if(!Q||!Q->bEnabled)return;
  auto* Runtime=GetWorld()->GetGameInstance()->GetSubsystem<US1QuestSubsystem>();if(!Runtime)return;
  const uint64 Run=DialogueRun;const int32 Offer=DataSession.CurrentNodeId;const TWeakObjectPtr<AS1QuestNpc> OriginalNpc=Npc;
  const TWeakObjectPtr<US1DialogueSubsystem> Weak=this;
  Runtime->RequestAccept(Q->QuestId,Q->DefinitionVersion,Npc->NpcId,[Weak,Run,Offer,OriginalNpc]
  {
   auto* Self=Weak.Get();auto* N=OriginalNpc.Get();
   if(!Self||!N||Self->Phase!=EPhase::Active||Self->DialogueRun!=Run||Self->ActiveNpc!=OriginalNpc||Self->DataSession.CurrentNodeId!=Offer)return;
   const auto* Current=FS1QuestData::FindNode(N->DialogueNodes,Offer);
   if(!Current||Current->Kind!=ES1DialogueNodeKind::QuestOffer||!FS1QuestData::FindNode(N->DialogueNodes,Current->AcceptNodeId))return;
   Self->DataSession.CurrentNodeId=Current->AcceptNodeId;Self->RefreshDialogueNode();
  });
  RefreshDialogueNode();return;
 }
 if(DataSession.Advance(Npc->DialogueNodes))RefreshDialogueNode();
}
void US1DialogueSubsystem::SecondaryDialogueAction()
{
 if(Phase!=EPhase::Active)return;
 auto* Npc=ActiveNpc.Get();if(!Npc)return;
 const auto* Node=FS1QuestData::FindNode(Npc->DialogueNodes,DataSession.CurrentNodeId);
 if(Node&&Node->Kind==ES1DialogueNodeKind::QuestOffer&&DataSession.ChoosePreview(Npc->DialogueNodes,false,true))RefreshDialogueNode();
 else CloseDialogue();
}
void US1DialogueSubsystem::RestoreView()
{
 RestorePrivatePresentation();
 if(auto* PC=OwnerPC.Get())
 {
  // Never overwrite a view target installed by death/travel/another owner.
  const bool bOwnsView=bCameraChanged?PC->GetViewTarget()==DialogueCamera:PC->GetViewTarget()==SavedViewTarget.Get();
  if(bCameraChanged&&PC->GetViewTarget()==DialogueCamera)
  {AActor* Target=SavedViewTarget.IsValid()?SavedViewTarget.Get():PC->GetPawn();if(IsValid(Target))PC->SetViewTarget(Target);}
  if(bOwnsView&&PC->GetPawn()==OwnerPawn.Get())PC->SetControlRotation(SavedControlRotation);
 }
 bCameraChanged=false;
 if(DialogueCamera){DialogueCamera->Destroy();DialogueCamera=nullptr;}
}
void US1DialogueSubsystem::FinishDialogue()
{
 if(!IsModal())return;
 ++DialogueRun; // Invalidates a late acceptance callback even if same NPC is reopened.
 RestoreView();
 if(DialogueWidget){DialogueWidget->OnClose.Unbind();DialogueWidget->OnPrimary.Unbind();DialogueWidget->OnSecondary.Unbind();DialogueWidget->OnAdvanceLine.Unbind();DialogueWidget->RemoveFromParent();DialogueWidget=nullptr;}
 DataSession.Reset();
 if(auto* PC=OwnerPC.Get())
 {
  if(PC->PlayerCameraManager)PC->PlayerCameraManager->StopCameraFade();
  PC->SetIgnoreMoveInput(false);PC->SetIgnoreLookInput(false); // balanced, not ResetIgnore* (other owners may exist).
  const auto* Network=GetWorld()->GetGameInstance()?GetWorld()->GetGameInstance()->GetSubsystem<US1NetworkSubsystem>():nullptr;
  const auto* Flow=GetWorld()->GetGameInstance()?GetWorld()->GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>():nullptr;
  if(PC->GetPawn()==OwnerPawn.Get()&&IsValid(PC->GetPawn())&&!GetWorld()->bIsTearingDown&&
   Network&&Network->IsConnected()&&Flow&&Flow->IsMainLevelReady())
  {PC->bShowMouseCursor=bSavedCursor;PC->SetInputMode(FInputModeGameOnly());}
 }
 OwnerPC.Reset();OwnerPawn.Reset();ActiveNpc.Reset();SavedViewTarget.Reset();Phase=EPhase::Closed;PhaseTime=0;
 UE_LOG(LogTemp,Log,TEXT("Dialogue closed. Local camera/input lease released."));
}
void US1DialogueSubsystem::AbortDialogue(){FinishDialogue();}
void US1DialogueSubsystem::NotifyLocalDamage(){LastDamageAt=FPlatformTime::Seconds();AbortDialogue();}
void US1DialogueSubsystem::Tick(float Delta)
{
 if(IsTemplate()||!GetWorld()||!GetWorld()->GetGameInstance())return;
 if(IsModal())
 {
  auto* PC=OwnerPC.Get();auto* Player=OwnerPawn.Get();auto* Npc=ActiveNpc.Get();
  const auto* Network=GetWorld()->GetGameInstance()->GetSubsystem<US1NetworkSubsystem>();
  const auto* Flow=GetWorld()->GetGameInstance()->GetSubsystem<US1LevelFlowSubsystem>();
  if(!PC||!Player||PC->GetPawn()!=Player||!Npc||!Network||!Network->IsConnected()||!Flow||!Flow->IsMainLevelReady()||
   !CanTalk(Player,Npc,Npc->ExitRadius)||Player->GetCharacterMovement()->MovementMode==MOVE_None||HasCombatThreat()||
   (bCameraChanged&&PC->GetViewTarget()!=DialogueCamera))AbortDialogue();
  else
  {
   if(bPrivatePresentation)ApplyPrivatePresentation();
   if(Phase==EPhase::Active)RefreshDialogueNode(); // Enable/disable on async server state, not only clicks.
   PhaseTime+=Delta;
   if(PhaseTime>=FadeDuration)
   {
    if(Phase==EPhase::FadeToDialogue)RevealDialogue();
    else if(Phase==EPhase::RevealDialogue)Phase=EPhase::Active;
    else if(Phase==EPhase::FadeToGame)
    {RestoreView();if(PC->PlayerCameraManager)PC->PlayerCameraManager->StartCameraFade(1,0,FadeDuration,FLinearColor::Black,false,false);Phase=EPhase::RevealGame;PhaseTime=0;}
    else if(Phase==EPhase::RevealGame)FinishDialogue();
   }
  }
 }
 RefreshCandidates();
}
void US1DialogueSubsystem::Deinitialize(){AbortDialogue();Super::Deinitialize();}
