#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/S1InventoryWidget.h"
#include "S1InventoryStackWidget.generated.h"
class UButton;
class UTextBlock;
DECLARE_DELEGATE_OneParam(FS1InventoryStackSelected,uint64);

/** Passive selectable stack row. Its identifier is a view selection, never authorization. */
UCLASS()
class S1_API US1InventoryStackWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetEntry(const FS1InventoryEntryView& Entry,bool bSelected,bool bSelectable);
 uint64 GetItemId() const {return ViewEntry.ItemId;}
 FS1InventoryStackSelected OnSelected;
protected:
 virtual void NativeConstruct() override;
 virtual void NativePreConstruct() override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> SelectButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> EntryText;
private:
 void RefreshView();
 UFUNCTION() void HandleSelected();
 FS1InventoryEntryView ViewEntry;
 bool bSelected=false,bSelectable=false;
};
