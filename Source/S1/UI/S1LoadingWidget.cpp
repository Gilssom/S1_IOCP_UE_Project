#include "UI/S1LoadingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void US1LoadingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("Background"));
	Background->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.03f, 1.0f));
	Background->SetPadding(FMargin(48.0f));
	Background->SetHorizontalAlignment(HAlign_Center);
	Background->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Background;

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("LoadingContent"));
	Background->SetContent(Content);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("LoadingTitle"));
	Title->SetText(NSLOCTEXT("S1", "LoadingTitle", "Loading..."));
	Title->SetJustification(ETextJustify::Center);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Content->AddChildToVerticalBox(Title);

	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(
		USpacer::StaticClass(), TEXT("LoadingSpacer"));
	Spacer->SetSize(FVector2D(480.0f, 24.0f));
	Content->AddChildToVerticalBox(Spacer);

	UProgressBar* Progress = WidgetTree->ConstructWidget<UProgressBar>(
		UProgressBar::StaticClass(), TEXT("LoadingProgress"));
	Progress->SetIsMarquee(true);
	Content->AddChildToVerticalBox(Progress);
}
