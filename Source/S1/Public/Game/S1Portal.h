#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S1Portal.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class USceneComponent;

/** Placeable presentation and overlap volume; travel is connected in a later server-authoritative step. */
UCLASS(Blueprintable)
class S1_API AS1Portal : public AActor
{
 GENERATED_BODY()
public:
 AS1Portal();
 virtual void BeginPlay() override;

 UFUNCTION()
 void OnPortalOverlap(UPrimitiveComponent* Overlapped,AActor* OtherActor,UPrimitiveComponent* OtherComponent,
  int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);

 void CheckPortalOverlap();
 FTimerHandle OverlapRetryTimer;

 // A future server-owned portal definition will assign and validate this identifier.
 UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="S1|Portal", meta=(ClampMin="0"))
 int32 PortalId = 0;

 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S1|Portal")
 TObjectPtr<USceneComponent> PortalRoot;

 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S1|Portal")
 TObjectPtr<UBoxComponent> TriggerVolume;

 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="S1|Portal")
 TObjectPtr<UNiagaraComponent> PortalEffect;
};
