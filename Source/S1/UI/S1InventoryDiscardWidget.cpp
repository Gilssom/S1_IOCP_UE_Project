#include "UI/S1InventoryDiscardWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SpinBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
void US1InventoryDiscardWidget::SetSourceCount(uint32 Count)
{Maximum=FMath::Clamp<uint32>(Count,1,99);Amount=1;bFinal=Count==1;bSubmitted=false;RefreshStage();}
void US1InventoryDiscardWidget::RefreshStage()
{
 if(AmountInput){AmountInput->SetMinValue(1);AmountInput->SetMaxValue(Maximum);AmountInput->SetDelta(1);AmountInput->SetValue(Amount);AmountInput->SetVisibility(bFinal?ESlateVisibility::Collapsed:ESlateVisibility::Visible);}
 if(DialogTitle)DialogTitle->SetText(FText::FromString(TEXT("아이템 필드에 버리기")));
 if(DialogHelp)DialogHelp->SetText(FText::FromString(TEXT("누구나 G로 전량 획득 · 30초 후 소멸 · Esc / I 취소")));
 if(MessageText)MessageText->SetText(FText::FromString(bFinal?FString::Printf(TEXT("정말 포션 %u개를 필드에 버리시겠습니까?\n누구나 획득할 수 있으며 30초 후 사라집니다."),Amount):FString::Printf(TEXT("필드에 버릴 수량을 선택하세요. (1 ~ %u)"),Maximum)));
 if(ConfirmText)ConfirmText->SetText(FText::FromString(bFinal?TEXT("예, 버리기"):TEXT("다음")));
 FocusSafeDefault();
}
void US1InventoryDiscardWidget::FocusSafeDefault()
{
 if(!bFinal)return;
 if(CancelButton)CancelButton->SetKeyboardFocus();
 if(CancelSlate.IsValid()&&FSlateApplication::IsInitialized())FSlateApplication::Get().SetKeyboardFocus(CancelSlate);
}
void US1InventoryDiscardWidget::NativeConstruct()
{Super::NativeConstruct();SetIsFocusable(true);if(ConfirmButton)ConfirmButton->OnClicked.AddUniqueDynamic(this,&US1InventoryDiscardWidget::ConfirmClick);if(CancelButton)CancelButton->OnClicked.AddUniqueDynamic(this,&US1InventoryDiscardWidget::CancelClick);RefreshStage();}
void US1InventoryDiscardWidget::NativeDestruct()
{if(ConfirmButton)ConfirmButton->OnClicked.RemoveDynamic(this,&US1InventoryDiscardWidget::ConfirmClick);if(CancelButton)CancelButton->OnClicked.RemoveDynamic(this,&US1InventoryDiscardWidget::CancelClick);Super::NativeDestruct();}
void US1InventoryDiscardWidget::ConfirmClick()
{
 if(bSubmitted)return;
 if(!bFinal)
 {
  const float Value=AmountInput?AmountInput->GetValue():static_cast<float>(Amount);
  if(!FMath::IsFinite(Value)||Value<1||Value>Maximum||Value!=FMath::FloorToFloat(Value))return;
  Amount=static_cast<uint32>(Value);bFinal=true;RefreshStage();return;
 }
 bSubmitted=true;OnConfirm.ExecuteIfBound(Amount);
}
void US1InventoryDiscardWidget::CancelClick(){if(!bSubmitted)OnCancel.ExecuteIfBound();}
FReply US1InventoryDiscardWidget::NativeOnPreviewKeyDown(const FGeometry&,const FKeyEvent& Event)
{if(Event.GetKey()==EKeys::Escape||Event.GetKey()==EKeys::I){CancelClick();return FReply::Handled();}if(Event.GetKey()==EKeys::Enter){return FReply::Handled();}return FReply::Unhandled();}
FReply US1InventoryDiscardWidget::NativeOnMouseButtonDown(const FGeometry&,const FPointerEvent&){return FReply::Handled();}
TSharedRef<SWidget> US1InventoryDiscardWidget::RebuildWidget()
{
 if(WidgetTree&&WidgetTree->RootWidget)return Super::RebuildWidget();
 return SNew(SOverlay)
 +SOverlay::Slot()[SNew(SBorder).BorderBackgroundColor(FLinearColor(0,0,0,.6f))]
 +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
 [SNew(SBox).WidthOverride(360)[SNew(SBorder).Padding(24)[SNew(SVerticalBox)
 +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text_Lambda([this]{return FText::FromString(bFinal?FString::Printf(TEXT("정말 포션 %u개를 필드에 버리시겠습니까?\n누구나 획득할 수 있으며 30초 후 사라집니다."),Amount):TEXT("필드에 버릴 수량을 선택하세요."));})]
 +SVerticalBox::Slot().AutoHeight().Padding(0,12)[SNew(SSpinBox<int32>).MinValue(1).MaxValue(static_cast<int32>(Maximum)).Value_Lambda([this]{return static_cast<int32>(Amount);}).Visibility_Lambda([this]{return bFinal?EVisibility::Collapsed:EVisibility::Visible;}).OnValueChanged_Lambda([this](int32 V){Amount=FMath::Clamp(V,1,static_cast<int32>(Maximum));})]
 +SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
 +SHorizontalBox::Slot()[SNew(SButton).Text_Lambda([this]{return FText::FromString(bFinal?TEXT("예, 버리기"):TEXT("다음"));}).OnClicked_Lambda([this]{ConfirmClick();return FReply::Handled();})]
 +SHorizontalBox::Slot()[SAssignNew(CancelSlate,SButton).Text(FText::FromString(TEXT("아니오 / 취소"))).OnClicked_Lambda([this]{CancelClick();return FReply::Handled();})]]]]];
}
