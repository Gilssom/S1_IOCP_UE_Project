#pragma once
#include "CoreMinimal.h"
#include "UI/S1QuickSlotWidget.h"
#include "Combat/S1KnightCombatComponent.h"
#include "S1MainHUDWidget.generated.h"
class UProgressBar;
class UButton;
class UBorder;

/** One local HUD, extending the existing inventory-owned quick-slot presentation. */
UCLASS()
class S1_API US1MainHUDWidget:public US1QuickSlotWidget
{
 GENERATED_BODY()
public:
 void SetCombatView(uint32 HP,uint32 Maximum,bool bDead,ES1KnightActionState Action,int32 Combo);
 void SetLevelView(uint32 Level);
protected:
 virtual void NativeConstruct() override;
 virtual void NativeDestruct() override;
 virtual void NativePreConstruct() override;
 virtual void NativeTick(const FGeometry& Geometry,float DeltaTime) override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UProgressBar> HealthBar;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> HealthText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> LevelText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> PrimaryText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> ChargeText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UBorder> PrimaryFrame;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UBorder> ChargeFrame;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> InventoryButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> QuestButton;
private:
 friend class FS1MainHUDTest;
 UFUNCTION() void InventoryClicked();
 UFUNCTION() void QuestClicked();
};
