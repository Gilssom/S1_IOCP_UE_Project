#include "UI/S1DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Colors/SSimpleGradient.h"

TSharedRef<SWidget> US1DialogueWidget::RebuildWidget()
{
 // Cosmetic shading behind the editable WBP tree, never an opaque camera crop.
 const auto EdgeHeight=TAttribute<float>::CreateLambda([this]{return FMath::Clamp(EdgeShadeHeightFraction,0.f,.25f);});
 const auto ClearHeight=TAttribute<float>::CreateLambda([this]{return 1.f-2.f*FMath::Clamp(EdgeShadeHeightFraction,0.f,.25f);});
 const auto Shade=[this]{return FLinearColor(0,0,0,FMath::Clamp(EdgeShadeOpacity,0.f,1.f));};
 const TSharedRef<SWidget> Content=Super::RebuildWidget();
 return SNew(SOverlay)
 +SOverlay::Slot()
 [SNew(SVerticalBox).Visibility(EVisibility::HitTestInvisible)
  +SVerticalBox::Slot().FillHeight(EdgeHeight)
  // Slate names the stop-line orientation: horizontal stops blend top-to-bottom.
  [SNew(SSimpleGradient).StartColor_Lambda(Shade).EndColor(FLinearColor::Transparent).Orientation(Orient_Horizontal)]
  +SVerticalBox::Slot().FillHeight(ClearHeight)[SNew(SSpacer)]
  +SVerticalBox::Slot().FillHeight(EdgeHeight)
  [SNew(SSimpleGradient).StartColor(FLinearColor::Transparent).EndColor_Lambda(Shade).Orientation(Orient_Horizontal)]]
 +SOverlay::Slot()[Content];
}

void US1DialogueWidget::NativeConstruct()
{
 Super::NativeConstruct();SetIsFocusable(true);
 // Stage A has no request contract. Never fake successful acceptance.
 if(AcceptButton)AcceptButton->SetIsEnabled(false);
 if(AcceptButton)AcceptButton->OnClicked.AddUniqueDynamic(this,&US1DialogueWidget::PrimaryClicked);
 if(DeclineButton)DeclineButton->OnClicked.AddUniqueDynamic(this,&US1DialogueWidget::SecondaryClicked);
}
void US1DialogueWidget::NativeDestruct()
{
 if(AcceptButton)AcceptButton->OnClicked.RemoveDynamic(this,&US1DialogueWidget::PrimaryClicked);
 if(DeclineButton)DeclineButton->OnClicked.RemoveDynamic(this,&US1DialogueWidget::SecondaryClicked);
 OnClose.Unbind();OnPrimary.Unbind();OnSecondary.Unbind();OnAdvanceLine.Unbind();Super::NativeDestruct();
}
void US1DialogueWidget::SetDialogue(const FText& Name,const FText& Body,const FText& Title,const FText& Preview)
{
 if(NpcNameText)NpcNameText->SetText(Name);
 if(DialogueText)DialogueText->SetText(Body);
 if(QuestTitleText)QuestTitleText->SetText(Title);
 if(QuestDetailText)QuestDetailText->SetText(Preview);
}
void US1DialogueWidget::CloseClicked(){OnClose.ExecuteIfBound();}
void US1DialogueWidget::SetReward(uint32 ItemId,uint32 Count)
{
 const bool bShow=ItemId!=0&&Count>0;
 if(RewardFrame)RewardFrame->SetVisibility(bShow?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
 const auto* Settings=GetDefault<US1PotionPresentationSettings>();
 if(RewardText)RewardText->SetText(FText::FromString(bShow?FString::Printf(TEXT("%s × %u"),
  ItemId==20001?*Settings->DisplayName:*FString::Printf(TEXT("아이템 %u"),ItemId),Count):FString()));
 if(RewardIcon)
 {
  RewardIcon->SetVisibility(bShow&&ItemId==20001?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
  if(bShow&&ItemId==20001)RewardIcon->SetBrushFromTexture(Cast<UTexture2D>(Settings->Icon.TryLoad()));
 }
}
void US1DialogueWidget::PrimaryClicked(){OnPrimary.ExecuteIfBound();}
void US1DialogueWidget::SecondaryClicked(){if(OnSecondary.IsBound())OnSecondary.Execute();else CloseClicked();}
void US1DialogueWidget::SetNavigation(const FText& Primary,const FText& Secondary,bool bEnabled,bool bShowQuest)
{
 if(AcceptLabel)AcceptLabel->SetText(Primary);
 if(DeclineLabel)DeclineLabel->SetText(Secondary);
 if(AcceptButton){AcceptButton->SetVisibility(Primary.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::Visible);AcceptButton->SetIsEnabled(bEnabled);}
 if(DeclineButton)DeclineButton->SetVisibility(Secondary.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::Visible);
 if(QuestDetailFrame)QuestDetailFrame->SetVisibility(bShowQuest?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
}
FReply US1DialogueWidget::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
 if(Event.GetKey()==EKeys::Escape){CloseClicked();return FReply::Handled();}
 return Super::NativeOnKeyDown(Geometry,Event);
}
FReply US1DialogueWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
 // Tunnel before a focused child button. Consume G even on Offer/End so it cannot
 // leak into pickup/interact; only a fresh press on a Line invokes the next action.
 if(Event.GetKey()==EKeys::G)
 {
  if(!Event.IsRepeat())OnAdvanceLine.ExecuteIfBound();
  return FReply::Handled();
 }
 return Super::NativeOnPreviewKeyDown(Geometry,Event);
}
TSharedRef<SWidget> US1NpcPromptWidget::RebuildWidget()
{
 SetIsFocusable(false);
 return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
 .BorderBackgroundColor(FLinearColor(.015f,.025f,.055f,.9f)).Padding(FMargin(12,6))
 [SNew(STextBlock).Text(FText::FromString(TEXT("[G] 대화")))
 .ColorAndOpacity(FLinearColor(.95f,.85f,.6f)).Font(FCoreStyle::GetDefaultFontStyle("Bold",18))];
}
