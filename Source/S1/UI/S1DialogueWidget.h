#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1DialogueWidget.generated.h"
class UTextBlock;
class UButton;
class UImage;
DECLARE_DELEGATE(FOnS1DialogueClose);

UCLASS()
class S1_API US1DialogueWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 FOnS1DialogueClose OnClose;
 FSimpleDelegate OnPrimary,OnSecondary;
 // Bound only for ordinary Line nodes, never a quest acceptance shortcut.
 FSimpleDelegate OnAdvanceLine;
 void SetDialogue(const FText& Name,const FText& Body,const FText& Title,const FText& Preview);
 void SetNavigation(const FText& PrimaryLabel,const FText& SecondaryLabel,bool bPrimaryEnabled,bool bShowQuest);
 void SetReward(uint32 ItemId,uint32 Count);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue|Edge Shade",meta=(ClampMin="0.0",ClampMax="1.0"))
 float EdgeShadeOpacity=.5f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue|Edge Shade",meta=(ClampMin="0.0",ClampMax="0.25"))
 float EdgeShadeHeightFraction=.08f;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void NativeConstruct() override;
 virtual void NativeDestruct() override;
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> NpcNameText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> DialogueText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> QuestTitleText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> QuestDetailText;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> AcceptButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UButton> DeclineButton;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> AcceptLabel;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> DeclineLabel;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UWidget> QuestDetailFrame;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UWidget> RewardFrame;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UImage> RewardIcon;
 UPROPERTY(meta=(BindWidgetOptional)) TObjectPtr<UTextBlock> RewardText;
private:
 friend class FS1DialogueInputTest;
 UFUNCTION() void CloseClicked();
 UFUNCTION() void PrimaryClicked();
 UFUNCTION() void SecondaryClicked();
};

/** Passive screen-space hint, never owns input. */
UCLASS()
class S1_API US1NpcPromptWidget : public UUserWidget
{
 GENERATED_BODY()
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
};
