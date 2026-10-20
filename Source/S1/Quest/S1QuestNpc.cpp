#include "Quest/S1QuestNpc.h"
#include "UI/S1DialogueWidget.h"
#include "UI/S1NpcNameplateWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Quest/S1QuestData.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "UObject/Package.h"

AS1QuestNpc::AS1QuestNpc()
{
 PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.1f;
 auto* Root=CreateDefaultSubobject<USceneComponent>(TEXT("Root"));SetRootComponent(Root);
 Visual=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Visual"));Visual->SetupAttachment(Root);
 Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);Visual->SetCanEverAffectNavigation(false);
 Prompt=CreateDefaultSubobject<UWidgetComponent>(TEXT("DialoguePrompt"));Prompt->SetupAttachment(Root);
 Prompt->SetRelativeLocation(FVector(0,0,210));Prompt->SetWidgetSpace(EWidgetSpace::Screen);
 Prompt->SetDrawSize(FVector2D(200,48));Prompt->SetPivot(FVector2D(.5,1));
 Prompt->SetWindowFocusable(false);Prompt->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Prompt->SetWidgetClass(US1NpcPromptWidget::StaticClass());Prompt->SetVisibility(false);
 Nameplate=CreateDefaultSubobject<US1NpcNameplateComponent>(TEXT("NpcNameplate"));Nameplate->SetupAttachment(Root);
 Nameplate->SetRelativeLocation(FVector(0,0,250));
 static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
 static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle(TEXT("/Game/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle"));
 NpcMesh=Mesh.Object;IdleAnimation=Idle.Object;
 DisplayName=FText::FromString(TEXT("경비대 안내원 (임시)"));
 Dialogue=FText::FromString(TEXT("주변을 순찰하고 있습니다. 도움이 필요하신가요?"));
 QuestTitle=FText::FromString(TEXT("퀘스트 준비 중"));
 QuestPreview=FText::FromString(TEXT("대화·카메라 기능 시험 단계입니다.\n\n스토리·목표·보상은 다음 단계에서 연결합니다.\n현재 수락 및 보상 지급은 지원하지 않습니다."));
 Tags.Add(TEXT("S1.Friendly"));
}
void AS1QuestNpc::ApplyAppearance()
{
 Visual->SetSkeletalMeshAsset(NpcMesh);
 Visual->SetRelativeLocation(MeshOffset);Visual->SetRelativeRotation(FRotator(0,MeshYaw,0));Visual->SetRelativeScale3D(MeshScale);
 Visual->SetAnimationMode(EAnimationMode::AnimationSingleNode);
 if(NpcMesh&&IdleAnimation&&NpcMesh->GetSkeleton()==IdleAnimation->GetSkeleton())Visual->PlayAnimation(IdleAnimation,true);
 else {Visual->SetAnimation(nullptr);if(IdleAnimation)UE_LOG(LogTemp,Warning,TEXT("NPC idle skeleton mismatch. NpcId[%d]"),NpcId);}
}
void AS1QuestNpc::OnConstruction(const FTransform& Transform){Super::OnConstruction(Transform);ApplyAppearance();RefreshNameplate();}
void AS1QuestNpc::BeginPlay(){Super::BeginPlay();ApplyAppearance();RefreshNameplate();ShowPrompt(false);UpdateNameplateRange();}
void AS1QuestNpc::Tick(float DeltaSeconds){Super::Tick(DeltaSeconds);UpdateNameplateRange();}
void AS1QuestNpc::UpdateNameplateRange()
{
 const auto* PC=GetWorld()?GetWorld()->GetFirstPlayerController():nullptr;
 Nameplate->UpdateForViewer(PC&&PC->IsLocalController()?PC->GetPawn():nullptr);
}
void AS1QuestNpc::ShowPrompt(bool bShow){Prompt->SetVisibility(bShow);}
void AS1QuestNpc::RefreshNameplate(){Nameplate->SetNpcName(DisplayName);}
void AS1QuestNpc::ExportQuestData()
{
#if WITH_EDITOR
 FString Text,Error;
 if(!FS1QuestData::ExportText(QuestDefinitions,DialogueNodes,Text,Error))
 {UE_LOG(LogTemp,Warning,TEXT("Quest export rejected: %s"),*Error);return;}
 const UWorld* World=GetWorld();
 if(HasAnyFlags(RF_ClassDefaultObject)||!World||World->WorldType!=EWorldType::Editor||
  World->GetOutermost()->GetName()!=TEXT("/Game/Maps/MainMap")||World->GetOutermost()->IsDirty())
 {UE_LOG(LogTemp,Warning,TEXT("Quest export rejected: use a placed NPC in saved MainMap (not CDO/PIE/dirty map)."));return;}
 TSet<int32> Required;
 for(const auto& Pair:QuestDefinitions->GetRowMap())
 {
  const auto& Q=*reinterpret_cast<const FS1QuestDefinitionRow*>(Pair.Value);
  Required.Add(Q.AcceptNpcId);Required.Add(Q.ReturnNpcId);
 }
 TMap<int32,const AS1QuestNpc*> Placement;
 for(TActorIterator<AS1QuestNpc> It(GetWorld());It;++It)
 {
  const AS1QuestNpc* N=*It;if(!Required.Contains(N->NpcId))continue;
  const FVector P=N->GetActorLocation();
  if(Placement.Contains(N->NpcId)||N->QuestDefinitions!=QuestDefinitions||N->DialogueNodes!=DialogueNodes||N->GetOutermost()->IsDirty()||
   P.ContainsNaN()||P.GetAbsMax()>10000000||!FMath::IsFinite(N->InteractionRadius)||N->InteractionRadius<50||N->InteractionRadius>2000)
  {UE_LOG(LogTemp,Warning,TEXT("Quest export rejected: duplicate/mixed/dirty/invalid NPC placement. NpcId[%d]"),N->NpcId);return;}
  Placement.Add(N->NpcId,N);
 }
 if(Placement.Num()!=Required.Num())
 {UE_LOG(LogTemp,Warning,TEXT("Quest export rejected: missing trusted NPC placement."));return;}
 Text.RemoveFromStart(TEXT("S1QUEST 1\n"));Text=TEXT("S1QUEST 2\n")+Text;
 Text+=FString::Printf(TEXT("NPCS %d\n"),Placement.Num());
 TArray<int32> Ids;Placement.GetKeys(Ids);Ids.Sort();
 for(int32 Id:Ids)
 {
  const auto* N=Placement.FindChecked(Id);const FVector P=N->GetActorLocation();
  Text+=FString::Printf(TEXT("N %d 1 %.9f %.9f %.9f %.9f 100 100 \"MainMap\"\n"),Id,P.X,P.Y,P.Z,N->InteractionRadius);
 }
 const FString Path=FPaths::ProjectSavedDir()/TEXT("QuestExports/QuestDefinitions.s1quest");
 IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);
 const FString Temporary=Path+TEXT(".tmp");
 if(!FFileHelper::SaveStringToFile(Text,*Temporary,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)||
  !IFileManager::Get().Move(*Path,*Temporary,true,true))
 {UE_LOG(LogTemp,Warning,TEXT("Quest export write failed; previous export was not intentionally replaced."));return;}
 UE_LOG(LogTemp,Display,TEXT("QuestExportComplete v2: %s (trusted saved NPC placements; server validation/deploy required)"),*FPaths::ConvertRelativePathToFull(Path));
#endif
}
