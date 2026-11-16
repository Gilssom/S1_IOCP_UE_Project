#include "Game/S1Portal.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Game/S1MyPlayer.h"
#include "S1GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AS1Portal::AS1Portal()
{
 PrimaryActorTick.bCanEverTick=false;
 bReplicates=false; // The external IOCP server owns region transitions.
 PortalRoot=CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
 SetRootComponent(PortalRoot);
 TriggerVolume=CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
 TriggerVolume->SetupAttachment(PortalRoot);
 TriggerVolume->SetBoxExtent(FVector(150.f,150.f,100.f));
 TriggerVolume->SetRelativeLocation(FVector(0.f,0.f,100.f));
 TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
 TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
 TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
 TriggerVolume->SetGenerateOverlapEvents(true);
 TriggerVolume->SetCanEverAffectNavigation(false);
 PortalEffect=CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalEffect"));
 PortalEffect->SetupAttachment(PortalRoot);
 PortalEffect->SetAutoActivate(true);
 PortalEffect->SetCanEverAffectNavigation(false);
 static ConstructorHelpers::FObjectFinder<UNiagaraSystem> Effect(TEXT("/Game/RPGEnvironmentVFX/VFX/Niagara/NS_TeleportZone.NS_TeleportZone"));
 if(Effect.Succeeded())PortalEffect->SetAsset(Effect.Object);
}

void AS1Portal::BeginPlay()
{
 Super::BeginPlay();
 if(TriggerVolume)TriggerVolume->OnComponentBeginOverlap.AddDynamic(this,&AS1Portal::OnPortalOverlap);
}

void AS1Portal::OnPortalOverlap(UPrimitiveComponent*,AActor* OtherActor,UPrimitiveComponent*,int32,bool,const FHitResult&)
{
 auto* Player=Cast<AS1MyPlayer>(OtherActor);
 if(!Player||!Player->IsLocallyControlled())return;
 if(auto* Instance=GetGameInstance<US1GameInstance>())Instance->RequestPortalTransfer(PortalId);
 if(UWorld* World=GetWorld())World->GetTimerManager().SetTimer(OverlapRetryTimer,this,&AS1Portal::CheckPortalOverlap,1.f,true);
}

void AS1Portal::CheckPortalOverlap()
{
 auto* Player=Cast<AS1MyPlayer>(UGameplayStatics::GetPlayerPawn(this,0));
 if(!Player||!TriggerVolume||!TriggerVolume->IsOverlappingActor(Player))
 {
  if(UWorld* World=GetWorld())World->GetTimerManager().ClearTimer(OverlapRetryTimer);
  return;
 }
 if(auto* Instance=GetGameInstance<US1GameInstance>())Instance->RequestPortalTransfer(PortalId);
}
