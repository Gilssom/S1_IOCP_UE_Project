#include "Loot/S1LootActor.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AS1LootActor::AS1LootActor()
{
 PrimaryActorTick.bCanEverTick=true;
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
 PickupHint=CreateDefaultSubobject<UTextRenderComponent>(TEXT("PickupHint"));
 PickupHint->SetupAttachment(SceneRoot);
 PickupHint->SetRelativeLocation(FVector(0,0,70));
 PickupHint->SetHorizontalAlignment(EHTA_Center);
 PickupHint->SetWorldSize(14.f);
 PickupHint->SetVisibility(false);
 PickupHint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 PickupHint->SetCanEverAffectNavigation(false);
 SetActorEnableCollision(false);
}

bool AS1LootActor::ApplyServerDrop(const FString& InDropKey,uint64 InOwnerCharacterId,
 uint32 InCount,const FVector& ServerFloorPosition)
{
 if(bRetired || InDropKey.Len()!=32 || !InOwnerCharacterId || InCount==0 ||
  ServerFloorPosition.ContainsNaN()) return false;
 for(TCHAR Character:InDropKey) if(!FChar::IsHexDigit(Character)) return false;
 // All-zero IDs are not legitimate drop identities.
 if(InDropKey==FString::ChrN(32,TEXT('0'))) return false;
 if(!DropKey.IsEmpty() && (DropKey!=InDropKey || OwnerCharacterId!=InOwnerCharacterId || Count!=InCount)) return false;
 DropKey=InDropKey;OwnerCharacterId=InOwnerCharacterId;Count=InCount;
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
 const bool Visible=!bRetired && !DropKey.IsEmpty() && bSelected && LocalCharacterId!=0 && LocalCharacterId==OwnerCharacterId;
 PickupHint->SetVisibility(Visible);
 if(Visible)
 {
  const FString Text=bPending?TEXT("획득 처리 중…"):FString::Printf(TEXT("G %s x%u 획득"),
   *GetDefault<US1PotionPresentationSettings>()->DisplayName,Count);
  PickupHint->SetText(FText::FromString(Text));
 }
}

bool AS1LootActor::IsPickupHintVisible() const {return PickupHint && PickupHint->IsVisible();}

void AS1LootActor::Tick(float DeltaSeconds)
{
 Super::Tick(DeltaSeconds);
 if(IsPickupHintVisible())
  if(APlayerController* PC=GetWorld()->GetFirstPlayerController())
   if(PC->PlayerCameraManager)
    PickupHint->SetWorldRotation((PC->PlayerCameraManager->GetCameraLocation()-PickupHint->GetComponentLocation()).Rotation());
 // No client UTC/countdown-driven removal or auto item addition.
}

void AS1LootActor::RetirePresentation()
{
 if(bRetired) return;
 bRetired=true;
 PickupHint->SetVisibility(false);
 PickupHint->SetText(FText::GetEmpty());
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
