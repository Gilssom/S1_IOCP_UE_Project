#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/S1InventoryWidget.h"
#include "S1InventorySlotWidget.generated.h"
class UImage;
class UTextBlock;
DECLARE_DELEGATE_OneParam(FS1SlotIntent,uint64);
UCLASS()
class S1_API US1InventorySlotWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetSlot(const FS1InventoryEntryView& Entry,bool bAvailable,bool bLocked,US1InventoryWidget* Source);
 FS1SlotIntent OnSelected,OnUseRequested;
 uint64 GetItemId() const {return Entry.ItemId;}
protected:
 virtual void NativeConstruct() override;
 virtual void NativePreConstruct() override;
 virtual FReply NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&) override;
 virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry&,const FPointerEvent&) override;
 virtual void NativeOnDragDetected(const FGeometry&,const FPointerEvent&,UDragDropOperation*&) override;
 virtual bool NativeOnDrop(const FGeometry&,const FDragDropEvent&,UDragDropOperation*) override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> ItemIcon;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> CountText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> SlotText;
private:
 friend class FS1InventoryGridTest;
 void Refresh();
 FS1InventoryEntryView Entry;
 bool bAvailable=false,bLocked=false;
 UPROPERTY(Transient) TWeakObjectPtr<US1InventoryWidget> SourceWindow;
};
