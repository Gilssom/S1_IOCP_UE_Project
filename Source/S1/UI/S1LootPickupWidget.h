#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1LootPickupWidget.generated.h"
class UTextBlock;

/** Passive local prompt. Does not submit pickup requests or decide ownership. */
UCLASS()
class S1_API US1LootPickupWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetPickupView(const FText& Name,uint32 Count,bool bPending);
protected:
 virtual void NativeConstruct() override;
 virtual void NativePreConstruct() override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> KeyText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> ItemText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> StatusText;
private:
 friend class FS1LootPickupWidgetTest;
 void RefreshView();
 FText ItemName;
 uint32 Quantity=1;
 bool bRequestPending=false;
};
