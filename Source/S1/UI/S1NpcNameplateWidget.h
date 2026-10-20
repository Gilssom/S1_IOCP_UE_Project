#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "S1NpcNameplateWidget.generated.h"
class UTextBlock;

/** Passive name view. Designer owns geometry/style; NPC owns the displayed name. */
UCLASS()
class S1_API US1NpcNameplateWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetNpcName(const FText& Name);
protected:
 virtual void NativeConstruct() override;
 virtual void NativePreConstruct() override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> NameText;
private:
 FText NpcName;
 void RefreshView();
};

/** Retains data even when the screen-space widget is created/recreated later. */
UCLASS(ClassGroup=UI,meta=(BlueprintSpawnableComponent))
class S1_API US1NpcNameplateComponent : public UWidgetComponent
{
 GENERATED_BODY()
public:
 US1NpcNameplateComponent();
 virtual void InitWidget() override;
 void SetNpcName(const FText& Name);
 void UpdateForViewer(const AActor* Viewer);
 static bool ShouldShowAtDistance(bool bWasVisible,float DistanceSquared,float ShowCm,float HideCm);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Nameplate",meta=(ClampMin="0",Units="cm")) float ShowDistance=1500.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Nameplate",meta=(ClampMin="0",Units="cm")) float HideDistance=1800.f;
private:
 FText NpcName;
 bool bWithinRange=false;
 void RefreshView();
};
