#include "Loot/S1LootActor.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "UI/S1LootPickupWidget.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/PackageName.h"

AS1LootActor::AS1LootActor()
{
 PrimaryActorTick.bCanEverTick=false;
 bReplicates=false; // External IOCP is authoritative; no parallel UE replication.
 SceneRoot=CreateDefaultSubobject<USceneComponent>(TEXT("LootRoot"));
 SetRootComponent(SceneRoot);
 PotionMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotionMesh"));
 PotionMesh->SetupAttachment(SceneRoot);
 PotionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 PotionMesh->SetGenerateOverlapEvents(false);
 PotionMesh->SetCanEverAffectNavigation(false);
 DropEffect=CreateDefaultSubobject<UNiagaraComponent>(TEXT("DropEffect"));
 DropEffect->SetupAttachment(SceneRoot);
 DropEffect->SetAutoActivate(false);
 DropEffect->SetAutoDestroy(false);
 DropEffect->SetCanEverAffectNavigation(false);
 PickupHint=CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupHintWidget"));
 PickupHint->SetupAttachment(SceneRoot);
 PickupHint->SetRelativeLocation(FVector(0,0,70));
 PickupHint->SetWidgetSpace(EWidgetSpace::Screen);
 PickupHint->SetDrawAtDesiredSize(true);
 PickupHint->SetPivot(FVector2D(.5f,1.f));
 PickupHint->SetWindowFocusable(false);
 PickupHint->SetTickWhenOffscreen(false);
 // Before first authoring there is no package; fresh runtime/cook anchors the
 // saved exact WBP class, without folder-wide always-cook settings.
 if(FPackageName::DoesPackageExist(TEXT("/Game/S1/UI/WBP_S1LootPickupPrompt")))
 {
  static ConstructorHelpers::FClassFinder<US1LootPickupWidget> Prompt(TEXT("/Game/S1/UI/WBP_S1LootPickupPrompt"));
  PickupWidgetClass=Prompt.Class;
 }
 PickupHint->SetVisibility(false);
 PickupHint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 PickupHint->SetCanEverAffectNavigation(false);
 SetActorEnableCollision(false);
}

bool AS1LootActor::ApplyServerDrop(const FString& InDropKey,uint64 InOwnerCharacterId,
 uint32 InCount,const FVector& ServerFloorPosition,bool bInPublic)
{
 if(bRetired || InDropKey.Len()!=32 || !InOwnerCharacterId || InCount==0 || InCount>99 ||
  ServerFloorPosition.ContainsNaN()) return false;
 for(TCHAR Character:InDropKey) if(!FChar::IsHexDigit(Character)) return false;
 // All-zero IDs are not legitimate drop identities.
 if(InDropKey==FString::ChrN(32,TEXT('0'))) return false;
 if(!DropKey.IsEmpty() && (DropKey!=InDropKey || OwnerCharacterId!=InOwnerCharacterId || Count!=InCount || bPublic!=bInPublic)) return false;
 DropKey=InDropKey;OwnerCharacterId=InOwnerCharacterId;Count=InCount;bPublic=bInPublic;
 SetActorLocation(ServerFloorPosition,false);
 if(HasActorBegunPlay()) ActivatePresentation();
 return true;
}

void AS1LootActor::BeginPlay()
{
 Super::BeginPlay();
 LoadPresentation();
 ActivatePresentation();
}

void AS1LootActor::LoadPresentation()
{
 const auto* Settings=GetDefault<US1PotionPresentationSettings>();
 const float HintHeight=FMath::IsFinite(Settings->PickupHintHeight)?Settings->PickupHintHeight:70.f;
 PickupHint->SetRelativeLocation(FVector(0,0,HintHeight));
 if(PickupWidgetClass)PickupHint->SetWidgetClass(PickupWidgetClass);
 if(auto* PC=GetWorld()->GetFirstPlayerController())PickupHint->SetOwnerPlayer(PC->GetLocalPlayer());
 PickupHint->InitWidget();
 if(auto* View=Cast<US1LootPickupWidget>(PickupHint->GetUserWidgetObject()))
  View->SetRenderTranslation(Settings->PickupHintScreenOffset.ContainsNaN()?FVector2D::ZeroVector:Settings->PickupHintScreenOffset);
 const float Scale=FMath::IsFinite(Settings->MeshScale)&&Settings->MeshScale>0.f?Settings->MeshScale:0.4f;
 const float Offset=FMath::IsFinite(Settings->FloorOffset)?Settings->FloorOffset:0.f;
 PotionMesh->SetRelativeScale3D(FVector(Scale));
 PotionMesh->SetRelativeLocation(FVector(0,0,Offset));
 PotionMesh->SetStaticMesh(Cast<UStaticMesh>(Settings->WorldMesh.TryLoad()));
 const float FxScale=FMath::IsFinite(Settings->EffectScale)&&Settings->EffectScale>0.f?Settings->EffectScale:1.f;
 DropEffect->SetRelativeScale3D(FVector(FxScale));
 DropEffect->SetAsset(Cast<UNiagaraSystem>(Settings->DropEffect.TryLoad()));
 // First-drop synchronous content load is temporary; no network/DB work here.
 if(!PotionMesh->GetStaticMesh()) UE_LOG(LogTemp,Warning,TEXT("S1 loot mesh unavailable: %s"),*Settings->WorldMesh.ToString());
 if(!DropEffect->GetAsset()) UE_LOG(LogTemp,Warning,TEXT("S1 loot effect unavailable: %s"),*Settings->DropEffect.ToString());
}

void AS1LootActor::ActivatePresentation()
{
 if(bRetired || DropKey.IsEmpty() || bEffectActivated || !DropEffect || !DropEffect->GetAsset()) return;
 bEffectActivated=true;
 DropEffect->Activate(true);
 // Never restart the effect on duplicate snapshots. Imported lifetime is not overridden.
}

void AS1LootActor::ShowOwnerPickupHint(uint64 LocalCharacterId,bool bSelected,bool bPending)
{
 const bool Visible=!bRetired && !DropKey.IsEmpty() && bSelected && LocalCharacterId!=0 && (bPublic||LocalCharacterId==OwnerCharacterId);
 PickupHint->SetVisibility(Visible);
 if(Visible)
 {
  if(auto* View=Cast<US1LootPickupWidget>(PickupHint->GetUserWidgetObject()))
   View->SetPickupView(FText::FromString(GetDefault<US1PotionPresentationSettings>()->DisplayName),Count,bPending);
 }
}

bool AS1LootActor::IsPickupHintVisible() const {return PickupHint && PickupHint->IsVisible();}

void AS1LootActor::RetirePresentation()
{
 if(bRetired) return;
 bRetired=true;
 PickupHint->SetVisibility(false);
 PickupHint->SetWidget(nullptr);
 if(DropEffect)
 {
  DropEffect->DeactivateImmediate();
  DropEffect->DestroyComponent();
  DropEffect=nullptr;
 }
 PotionMesh->SetStaticMesh(nullptr);
 SetActorHiddenInGame(true);
 SetActorTickEnabled(false);
 DropKey.Empty();OwnerCharacterId=0;Count=0;
}

void AS1LootActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
 RetirePresentation();
 Super::EndPlay(EndPlayReason);
}
