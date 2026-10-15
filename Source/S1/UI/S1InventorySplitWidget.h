#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1InventorySplitWidget.generated.h"
class USpinBox;
class UButton;
class UTextBlock;
DECLARE_DELEGATE_OneParam(FS1SplitConfirm,uint32);
DECLARE_DELEGATE(FS1SplitCancel);
UCLASS()
class S1_API US1InventorySplitWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetSourceCount(uint32 Count);
 FS1SplitConfirm OnConfirm;
 FS1SplitCancel OnCancel;
protected:
 virtual void NativeConstruct() override;
 virtual void NativeDestruct() override;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
 virtual FReply NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&) override;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USpinBox> AmountInput;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ConfirmButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CancelButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> RangeText;
private:
 friend class FS1InventoryStackCoordinatorTest;
 UFUNCTION() void ConfirmClick();
 UFUNCTION() void CancelClick();
 void RefreshRange();
 uint32 Maximum=1;
 int32 Amount=1;
};
