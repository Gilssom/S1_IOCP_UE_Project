#include "UI/S1QuestJournalWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "InputCoreTypes.h"
#include "Components/Image.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Engine/Texture2D.h"
void US1QuestJournalWidget::NativeConstruct()
{
 Super::NativeConstruct();SetIsFocusable(true);
 if(CloseButton)CloseButton->OnClicked.AddUniqueDynamic(this,&US1QuestJournalWidget::CloseClicked);
 if(ActiveTabButton)ActiveTabButton->OnClicked.AddUniqueDynamic(this,&US1QuestJournalWidget::ActiveClicked);
 if(CompletedTabButton)CompletedTabButton->OnClicked.AddUniqueDynamic(this,&US1QuestJournalWidget::CompletedClicked);
 if(QuestListButton)QuestListButton->OnClicked.AddUniqueDynamic(this,&US1QuestJournalWidget::SelectClicked);
 Refresh();
}
void US1QuestJournalWidget::NativeDestruct()
{
 if(CloseButton)CloseButton->OnClicked.RemoveDynamic(this,&US1QuestJournalWidget::CloseClicked);
 if(ActiveTabButton)ActiveTabButton->OnClicked.RemoveDynamic(this,&US1QuestJournalWidget::ActiveClicked);
 if(CompletedTabButton)CompletedTabButton->OnClicked.RemoveDynamic(this,&US1QuestJournalWidget::CompletedClicked);
 if(QuestListButton)QuestListButton->OnClicked.RemoveDynamic(this,&US1QuestJournalWidget::SelectClicked);
 OnClose.Unbind();Super::NativeDestruct();
}
void US1QuestJournalWidget::SetQuest(const FS1QuestJournalView& InEntry,bool bInReady)
{
 if(Entry.QuestId!=InEntry.QuestId||Entry.bCompleted!=InEntry.bCompleted||!bInReady)bSelected=false;
 Entry=InEntry;bReady=bInReady;Refresh();
}
void US1QuestJournalWidget::ActiveClicked(){bCompletedTab=false;bSelected=false;Refresh();}
void US1QuestJournalWidget::CompletedClicked(){bCompletedTab=true;bSelected=false;Refresh();}
void US1QuestJournalWidget::SelectClicked()
{if(bReady&&Entry.QuestId&&Entry.bCompleted==bCompletedTab){bSelected=true;Refresh();}}
void US1QuestJournalWidget::Refresh()
{
 const bool bListed=bReady&&Entry.QuestId!=0&&Entry.bCompleted==bCompletedTab;
 const bool bDetail=bListed&&bSelected;
 if(ActiveTabButton)ActiveTabButton->SetBackgroundColor(bCompletedTab?FLinearColor(.38f,.42f,.48f):FLinearColor(1,.78f,.40f));
 if(CompletedTabButton)CompletedTabButton->SetBackgroundColor(bCompletedTab?FLinearColor(1,.78f,.40f):FLinearColor(.38f,.42f,.48f));
 if(QuestListButton){QuestListButton->SetVisibility(bListed?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
  QuestListButton->SetBackgroundColor(bDetail?FLinearColor(1,.83f,.53f):FLinearColor(.6f,.65f,.72f));}
 if(QuestListTitle)QuestListTitle->SetText(bListed?Entry.Title:FText::GetEmpty());
 if(EmptyListText)
 {
  EmptyListText->SetVisibility(bListed?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);
  EmptyListText->SetText(FText::FromString(!bReady?TEXT("퀘스트 동기화 확인 중"):bCompletedTab?TEXT("완료한 퀘스트가 없습니다."):TEXT("진행 중인 퀘스트가 없습니다.")));
 }
 if(SelectionHintText)SelectionHintText->SetVisibility(bDetail?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);
 if(DetailFrame)DetailFrame->SetVisibility(bDetail?ESlateVisibility::SelfHitTestInvisible:ESlateVisibility::Collapsed);
 if(QuestTitleText)QuestTitleText->SetText(bDetail?Entry.Title:FText::GetEmpty());
 if(QuestDetailText)QuestDetailText->SetText(bDetail?FText::FromString(Entry.Description.ToString()+TEXT("\n\n")+Entry.Objective.ToString()):FText::GetEmpty());
 if(QuestStateText)QuestStateText->SetText(bDetail?Entry.StateText:FText::GetEmpty());
 const bool bReward=bDetail&&Entry.RewardItemId&&Entry.RewardCount;
 if(RewardFrame)RewardFrame->SetVisibility(bReward?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
 const auto* Settings=GetDefault<US1PotionPresentationSettings>();
 if(RewardText)RewardText->SetText(bReward?FText::FromString(FString::Printf(TEXT("%s × %u"),
  Entry.RewardItemId==20001?*Settings->DisplayName:*FString::Printf(TEXT("아이템 %u"),Entry.RewardItemId),Entry.RewardCount)):FText::GetEmpty());
 if(RewardIcon)
 {
  RewardIcon->SetVisibility(bReward&&Entry.RewardItemId==20001?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
  if(bReward&&Entry.RewardItemId==20001)RewardIcon->SetBrushFromTexture(Cast<UTexture2D>(Settings->Icon.TryLoad()));
 }
}
void US1QuestJournalWidget::CloseClicked(){OnClose.ExecuteIfBound();}
FReply US1QuestJournalWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
 if(Event.GetKey()==EKeys::O||Event.GetKey()==EKeys::Escape)
 {if(!Event.IsRepeat())CloseClicked();return FReply::Handled();}
 return Super::NativeOnPreviewKeyDown(Geometry,Event);
}
