#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1QuestTrackerWidget.generated.h"
class UTextBlock;
UCLASS()
class S1_API US1QuestTrackerWidget:public UUserWidget
{
 GENERATED_BODY()
public:
 void SetQuestText(const FText& Text);
protected:
 virtual void NativeConstruct() override;
 UPROPERTY(meta=(BindWidgetOptional),BlueprintReadOnly) TObjectPtr<UTextBlock> QuestText;
};
