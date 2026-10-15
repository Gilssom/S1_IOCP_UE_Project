#include "UI/S1InventorySplitWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SpinBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"

void US1InventorySplitWidget::SetSourceCount(uint32 Count)
{Maximum=FMath::Clamp<uint32>(Count>1?Count-1:1,1,98);Amount=FMath::Max(1,static_cast<int32>(Count/2));RefreshRange();}
void US1InventorySplitWidget::RefreshRange()
{
 Amount=FMath::Clamp(Amount,1,static_cast<int32>(Maximum));
 if(AmountInput){AmountInput->SetMinValue(1);AmountInput->SetMaxValue(Maximum);AmountInput->SetMinSliderValue(1);AmountInput->SetMaxSliderValue(Maximum);AmountInput->SetDelta(1);AmountInput->SetValue(Amount);}
 if(RangeText)RangeText->SetText(FText::FromString(FString::Printf(TEXT("분할 수량: 1 ~ %u"),Maximum)));
}
void US1InventorySplitWidget::NativeConstruct()
{
 Super::NativeConstruct();SetIsFocusable(true);
 if(ConfirmButton)ConfirmButton->OnClicked.AddUniqueDynamic(this,&US1InventorySplitWidget::ConfirmClick);
 if(CancelButton)CancelButton->OnClicked.AddUniqueDynamic(this,&US1InventorySplitWidget::CancelClick);
 RefreshRange();
}
void US1InventorySplitWidget::NativeDestruct()
{
 if(ConfirmButton)ConfirmButton->OnClicked.RemoveDynamic(this,&US1InventorySplitWidget::ConfirmClick);
 if(CancelButton)CancelButton->OnClicked.RemoveDynamic(this,&US1InventorySplitWidget::CancelClick);
 Super::NativeDestruct();
}
void US1InventorySplitWidget::ConfirmClick()
{
 const float Value=AmountInput?AmountInput->GetValue():static_cast<float>(Amount);
 if(!FMath::IsFinite(Value)||Value<1||Value>Maximum||Value!=FMath::FloorToFloat(Value))
 {if(RangeText)RangeText->SetText(FText::FromString(TEXT("유효한 정수 수량을 입력해 주세요.")));return;}
 OnConfirm.ExecuteIfBound(static_cast<uint32>(Value));
}
void US1InventorySplitWidget::CancelClick(){OnCancel.ExecuteIfBound();}
FReply US1InventorySplitWidget::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& Event)
{
 if(Event.GetKey()==EKeys::Escape||Event.GetKey()==EKeys::I){CancelClick();return FReply::Handled();}
 if(Event.GetKey()==EKeys::Enter&&HasKeyboardFocus()){ConfirmClick();return FReply::Handled();}
 return FReply::Unhandled();
}
FReply US1InventorySplitWidget::NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&){return FReply::Handled();}
TSharedRef<SWidget> US1InventorySplitWidget::RebuildWidget()
{
 if(WidgetTree&&WidgetTree->RootWidget)return Super::RebuildWidget();
 return SNew(SOverlay)
 +SOverlay::Slot()[SNew(SBorder).BorderBackgroundColor(FLinearColor(0,0,0,0.6f))]
 +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
 [SNew(SBox).WidthOverride(280)[SNew(SBorder).Padding(16)
 [SNew(SVerticalBox)
 +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(FText::FromString(TEXT("아이템 분할")))]
 +SVerticalBox::Slot().AutoHeight().Padding(0,10)[SNew(SSpinBox<int32>).MinValue(1).MaxValue(static_cast<int32>(Maximum))
  .Value_Lambda([this]{return Amount;}).OnValueChanged_Lambda([this](int32 V){Amount=V;})]
 +SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
  +SHorizontalBox::Slot()[SNew(SButton).Text(FText::FromString(TEXT("확인"))).OnClicked_Lambda([this]{ConfirmClick();return FReply::Handled();})]
  +SHorizontalBox::Slot()[SNew(SButton).Text(FText::FromString(TEXT("취소"))).OnClicked_Lambda([this]{CancelClick();return FReply::Handled();})]]]]];
}
