#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S1LootActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class US1LootPickupWidget;
class UNiagaraComponent;

/** Server-drop presentation only. No overlap grants, local expiry, physics or replication authority. */
UCLASS(NotBlueprintable)
class S1_API AS1LootActor : public AActor
{
 GENERATED_BODY()
public:
 AS1LootActor();
 // Stable opaque16-byte server ID rendered as32hex. Position is server floor/pivot,
 // not monster capsule center. Metadata immutable for a given actor/drop identity.
 bool ApplyServerDrop(const FString& InDropKey,uint64 InOwnerCharacterId,
  uint32 InCount,const FVector& ServerFloorPosition,bool bInPublic=false);
 void ShowOwnerPickupHint(uint64 LocalCharacterId,bool bSelected,bool bPending);
 void RetirePresentation(); // WorldEntity calls only on server removal/world teardown.
 bool IsPickupHintVisible() const;
 FString GetDropKey() const {return DropKey;}
 bool IsRetired() const {return bRetired;}
protected:
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
 void LoadPresentation();
 void ActivatePresentation();
 UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> SceneRoot;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> PotionMesh;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UNiagaraComponent> DropEffect;
 UPROPERTY(VisibleAnywhere) TObjectPtr<UWidgetComponent> PickupHint;
 UPROPERTY() TSubclassOf<US1LootPickupWidget> PickupWidgetClass;
 FString DropKey;
 uint64 OwnerCharacterId=0;
 uint32 Count=0;
 bool bPublic=false;
 bool bRetired=false;
 bool bEffectActivated=false;
};
