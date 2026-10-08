#include "UI/S1InventoryStackWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void US1InventoryStackWidget::SetEntry(const FS1InventoryEntryView& Entry,bool bInSelected,bool bInSelectable)
{ViewEntry=Entry;bSelected=bInSelected;bSelectable=bInSelectable;RefreshView();}
void US1InventoryStackWidget::NativeConstruct()
{
 Super::NativeConstruct();
 if(SelectButton)SelectButton->OnClicked.AddUniqueDynamic(this,&US1InventoryStackWidget::HandleSelected);
 RefreshView();
}
void US1InventoryStackWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime()&&!ViewEntry.ItemId)
 {if(EntryText)EntryText->SetText(FText::FromString(TEXT("슬롯 1: 회복 포션 x5")));}
 else RefreshView();
}
void US1InventoryStackWidget::RefreshView()
{
 if(SelectButton)SelectButton->SetIsEnabled(bSelectable&&ViewEntry.ItemId!=0);
 if(EntryText)EntryText->SetText(FText::FromString(FString::Printf(TEXT("%s 슬롯 %u: 회복 포션 x%u"),
  bSelected?TEXT("▶"):TEXT("  "),ViewEntry.Slot+1,ViewEntry.Count)));
}
void US1InventoryStackWidget::HandleSelected()
{if(!IsDesignTime()&&bSelectable&&ViewEntry.ItemId)OnSelected.ExecuteIfBound(ViewEntry.ItemId);}
