#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1InventoryDiscardWidget.generated.h"
class USpinBox;
class UButton;
class UTextBlock;
class SButton;
DECLARE_DELEGATE_OneParam(FS1DiscardConfirm,uint32);
DECLARE_DELEGATE(FS1DiscardCancel);
/** Passive two-stage view. Never sends a packet or mutates stock. */
UCLASS()
class S1_API US1InventoryDiscardWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetSourceCount(uint32 Count);
 void FocusSafeDefault();
 FS1DiscardConfirm OnConfirm;
 FS1DiscardCancel OnCancel;
protected:
 virtual void NativeConstruct() override;
 virtual void NativeDestruct() override;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual FReply NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent&) override;
 virtual FReply NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&) override;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<USpinBox> AmountInput;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> ConfirmButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> CancelButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> MessageText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> ConfirmText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> DialogTitle;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> DialogHelp;
private:
 friend class FS1InventoryDiscardTest;
 UFUNCTION() void ConfirmClick();
 UFUNCTION() void CancelClick();
 void RefreshStage();
 uint32 Maximum=1,Amount=1;
 bool bFinal=false,bSubmitted=false;
 TSharedPtr<SButton> CancelSlate;
};
