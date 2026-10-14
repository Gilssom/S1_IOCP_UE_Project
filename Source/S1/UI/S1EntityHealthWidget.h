#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1EntityHealthWidget.generated.h"
class UProgressBar;
class UTextBlock;

/** Passive presentation only; never calculates damage or sends requests. */
UCLASS()
class S1_API US1EntityHealthWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetServerView(uint32 Current, uint32 Maximum, bool bDead, const FText& Name);
 float GetHealthFraction() const;
protected:
 virtual void NativeConstruct() override;
 virtual void NativePreConstruct() override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UProgressBar> HealthBar;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> HealthText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> NameText;
private:
 friend class FS1EntityHealthWidgetTest;
 void RefreshView();
 uint32 Hp=0, MaxHp=0;
 bool bIsDead=false;
 FText DisplayName;
};
