#include "UI/S1QuestTrackerWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
void US1QuestTrackerWidget::NativeConstruct()
{
 Super::NativeConstruct();SetIsFocusable(false);SetVisibility(ESlateVisibility::HitTestInvisible);
 if(!WidgetTree->RootWidget)
 {
  auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
  QuestText=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),TEXT("QuestText"));
  auto* CanvasSlot=Canvas->AddChildToCanvas(QuestText);CanvasSlot->SetAnchors(FAnchors(1,0));CanvasSlot->SetAlignment(FVector2D(1,0));
  CanvasSlot->SetPosition(FVector2D(-32,160));CanvasSlot->SetSize(FVector2D(320,140));QuestText->SetAutoWrapText(true);
 }
}
void US1QuestTrackerWidget::SetQuestText(const FText& Text)
{
 // Optional authored hierarchy: preserve the original single-text fallback.
 auto* Title=Cast<UTextBlock>(GetWidgetFromName(TEXT("TrackerTitleText")));
 auto* Objective=Cast<UTextBlock>(GetWidgetFromName(TEXT("TrackerObjectiveText")));
 auto* Status=Cast<UTextBlock>(GetWidgetFromName(TEXT("TrackerStatusText")));
 if(!Title||!Objective||!Status){if(QuestText)QuestText->SetText(Text);return;}
 if(QuestText){QuestText->SetText(Text);QuestText->SetVisibility(ESlateVisibility::Collapsed);}
 TArray<FString> Lines;Text.ToString().ParseIntoArrayLines(Lines,false);
 Title->SetText(FText::FromString(Lines.IsValidIndex(0)?Lines[0]:FString()));
 Objective->SetText(FText::FromString(Lines.IsValidIndex(1)?Lines[1]:FString()));
 FString Extra;for(int32 Index=2;Index<Lines.Num();++Index){if(!Extra.IsEmpty())Extra+=TEXT("\n");Extra+=Lines[Index];}
 Status->SetText(FText::FromString(Extra));
 Status->SetVisibility(Extra.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);
}
