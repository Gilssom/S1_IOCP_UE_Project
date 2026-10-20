#pragma once
#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "S1EntityHealthComponent.generated.h"
class US1EntityHealthWidget;

/** One passive screen-space bar per entity; cached snapshot survives late widget creation. */
UCLASS(ClassGroup=(UI),meta=(BlueprintSpawnableComponent))
class S1_API US1EntityHealthComponent : public UWidgetComponent
{
 GENERATED_BODY()
public:
 US1EntityHealthComponent();
 virtual void BeginPlay() override;
 virtual void InitWidget() override;
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 void SetServerView(uint32 Current,uint32 Maximum,bool bDead,const FText& Name);
 uint32 GetServerHP() const {return Hp;}
 uint32 GetServerMaxHP() const {return MaxHp;}
 bool IsServerDead() const {return bDead;}
 void SetPresentationEnabled(bool bEnabled);
 // Local dialogue mask; never replaces server HP or the normal visibility policy.
 void SetDialogueSuppressed(bool bSuppressed);
 bool IsDialogueSuppressed() const {return bDialogueSuppressed;}
 UPROPERTY(EditAnywhere,Category="HP|Presentation") bool bHideOnDeath=false;
private:
 void RefreshView();
 UPROPERTY(EditDefaultsOnly,Category="HP|Presentation") TSubclassOf<US1EntityHealthWidget> HealthWidgetClass;
 bool bPresentationEnabled=true;
 bool bDialogueSuppressed=false;
 bool bLastDialogueModal=false;
 uint32 Hp=0,MaxHp=0;
 bool bDead=false;
 FText DisplayName;
};
