#include "UI/S1InventorySlotWidget.h"
#include "UI/S1InventoryDragOperation.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Loot/S1InventorySubsystem.h"
#include "Engine/GameInstance.h"
void US1InventorySlotWidget::SetSlot(const FS1InventoryEntryView& Value,bool Available,bool Locked,US1InventoryWidget* Source)
{Entry=Value;bAvailable=Available;bLocked=Locked;SourceWindow=Source;Refresh();}
void US1InventorySlotWidget::NativeConstruct()
{Super::NativeConstruct();SetIsFocusable(false);Refresh();}
void US1InventorySlotWidget::NativePreConstruct()
{Super::NativePreConstruct();if(IsDesignTime()){Entry={1,5,0};bLocked=false;}Refresh();}
void US1InventorySlotWidget::Refresh()
{
 if(ItemIcon){ItemIcon->SetVisibility(Entry.ItemId?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);
  if(Entry.ItemId)ItemIcon->SetBrushFromTexture(Cast<UTexture2D>(GetDefault<US1PotionPresentationSettings>()->Icon.TryLoad()),false);}
 if(CountText)CountText->SetText(Entry.ItemId?FText::AsNumber(Entry.Count):FText::GetEmpty());
 if(SlotText)SlotText->SetText(bLocked?FText::FromString(TEXT("잠김")):FText::AsNumber(Entry.Slot+1));
 SetRenderOpacity(bLocked?.35f:1.f);
}
FReply US1InventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& Geometry,const FPointerEvent& Event)
{
 if(IsDesignTime()||Event.GetEffectingButton()!=EKeys::LeftMouseButton||!bAvailable||bLocked||!Entry.ItemId)return FReply::Handled();
 OnSelected.ExecuteIfBound(Entry.ItemId);
 return UWidgetBlueprintLibrary::DetectDragIfPressed(Event,this,EKeys::LeftMouseButton).NativeReply;
}
FReply US1InventorySlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& Geometry,const FPointerEvent& Event)
{
 if(!IsDesignTime()&&Event.GetEffectingButton()==EKeys::LeftMouseButton&&bAvailable&&!bLocked&&Entry.ItemId)
  OnUseRequested.ExecuteIfBound(Entry.ItemId);
 return FReply::Handled();
}
void US1InventorySlotWidget::NativeOnDragDetected(const FGeometry& Geometry,const FPointerEvent& Event,UDragDropOperation*& Out)
{
 Out=nullptr;if(IsDesignTime()||!bAvailable||bLocked||!Entry.ItemId||!SourceWindow.IsValid())return;
 auto* Drag=NewObject<US1InventoryDragOperation>();Drag->ItemId=Entry.ItemId;Drag->SourceWindow=SourceWindow;Drag->Pivot=EDragPivot::CenterCenter;
 Drag->SourceSlot=Entry.Slot;
 if(auto* GI=GetGameInstance())Drag->InventoryVersion=GI->GetSubsystem<US1InventorySubsystem>()->GetInventoryVersion();
 if(auto* Ghost=CreateWidget<US1InventorySlotWidget>(this,GetClass()))
 {Ghost->SetSlot(Entry,false,false,nullptr);Ghost->SetVisibility(ESlateVisibility::HitTestInvisible);Drag->DefaultDragVisual=Ghost;}
 Out=Drag;
 // Cancellation/outside drop never mutates inventory. Only a validated cell drop moves.
}
bool US1InventorySlotWidget::NativeOnDrop(const FGeometry&,const FDragDropEvent&,UDragDropOperation* Operation)
{
 auto* Drag=Cast<US1InventoryDragOperation>(Operation);
 if(IsDesignTime()||!Drag||bLocked||!bAvailable||Entry.ItemId||!SourceWindow.IsValid()||Drag->SourceWindow!=SourceWindow)return false;
 auto* GI=GetGameInstance();auto* Coordinator=GI?GI->GetSubsystem<US1InventorySubsystem>():nullptr;
 return Coordinator&&Coordinator->RequestMoveItem(Drag->ItemId,Drag->SourceSlot,Entry.Slot,Drag->InventoryVersion,SourceWindow.Get());
}
