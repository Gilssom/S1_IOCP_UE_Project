#include "UI/S1LootPickupWidget.h"
#include "Components/TextBlock.h"

void US1LootPickupWidget::SetPickupView(const FText& Name,uint32 Count,bool bPending)
{ItemName=Name;Quantity=Count;bRequestPending=bPending;RefreshView();}
void US1LootPickupWidget::NativeConstruct()
{Super::NativeConstruct();SetIsFocusable(false);SetVisibility(ESlateVisibility::HitTestInvisible);RefreshView();}
void US1LootPickupWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime())SetPickupView(FText::FromString(TEXT("회복 포션")),3,false);
 else RefreshView();
}
void US1LootPickupWidget::RefreshView()
{
 if(KeyText)KeyText->SetText(FText::FromString(TEXT("G")));
 if(ItemText)ItemText->SetText(FText::FromString(FString::Printf(TEXT("%s × %u"),*ItemName.ToString(),Quantity)));
 if(StatusText)StatusText->SetText(FText::FromString(bRequestPending?TEXT("획득 처리 중…"):TEXT("획득")));
 // Designer owns all fonts, colors and layout. UI never touches inventory/network.
}
