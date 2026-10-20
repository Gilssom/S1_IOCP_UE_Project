#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1QuestJournalWidget.generated.h"
class UTextBlock;
class UButton;
class UImage;

struct FS1QuestJournalView
{
 uint32 QuestId=0,RewardItemId=0,RewardCount=0;
 bool bCompleted=false;
 FText Title,Description,Objective,StateText;
};

/** Read-only quest journal. All entries originate from the owning quest cache. */
UCLASS()
class S1_API US1QuestJournalWidget:public UUserWidget
{
 GENERATED_BODY()
 friend class FS1QuestJournalTest;
public:
 FSimpleDelegate OnClose;
 void SetQuest(const FS1QuestJournalView& Entry,bool bReady);
protected:
 virtual void NativeConstruct() override;
 virtual void NativeDestruct() override;
 virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> ActiveTabButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> CompletedTabButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> QuestListButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> QuestListTitle;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> EmptyListText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> SelectionHintText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UWidget> DetailFrame;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> QuestTitleText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> QuestDetailText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> QuestStateText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UWidget> RewardFrame;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> RewardIcon;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> RewardText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> CloseButton;
private:
 UFUNCTION() void CloseClicked();
 UFUNCTION() void ActiveClicked();
 UFUNCTION() void CompletedClicked();
 UFUNCTION() void SelectClicked();
 void Refresh();
 FS1QuestJournalView Entry;
 bool bReady=false,bCompletedTab=false,bSelected=false;
};
