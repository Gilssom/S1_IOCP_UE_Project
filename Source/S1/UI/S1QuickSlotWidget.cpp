#include "UI/S1QuickSlotWidget.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Texture2D.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Loot/S1InventorySubsystem.h"
#include "UI/S1InventoryDragOperation.h"
#include "InputCoreTypes.h"
#include "Engine/GameInstance.h"

void US1QuickSlotWidget::SetView(const TArray<int32>& Slots,int64 Count,const FString& State,bool bReady)
{Definitions=Slots;Quantity=FMath::Max<int64>(0,Count);Status=State;bUsable=bReady;ApplyUMGView(Definitions,Quantity,Status,bUsable);}
void US1QuickSlotWidget::ApplyUMGView(const TArray<int32>& Slots,int64 Count,const FString& State,bool bReady)
{
 if(!Icon)Icon=Cast<UTexture2D>(GetDefault<US1PotionPresentationSettings>()->Icon.TryLoad());
 UImage* Images[4]={Icon1,Icon2,Icon3,Icon4};
 UTextBlock* Counts[4]={Count1,Count2,Count3,Count4};
 UTextBlock* States[4]={State1,State2,State3,State4};
 for(int32 I=0;I<4;++I)
 {
  const bool bAssigned=Slots.IsValidIndex(I)&&Slots[I]==20001;
  if(Images[I])
  {
   if(Images[I]->GetBrush().GetResourceObject()!=Icon)Images[I]->SetBrushFromTexture(Icon,false);
   Images[I]->SetVisibility(bAssigned?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);
   Images[I]->SetRenderOpacity(bReady&&Count>0?1.f:0.4f);
  }
  if(Counts[I])Counts[I]->SetText(FText::FromString(bAssigned?FString::Printf(TEXT("%lld"),FMath::Max<int64>(0,Count)):TEXT("")));
  if(States[I])States[I]->SetText(FText::FromString(bAssigned?State:FString()));
 }
}
void US1QuickSlotWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime())ApplyUMGView({20001,20001,20001,20001},DesignerPreviewCount,TEXT("사용 가능"),true);
 else ApplyUMGView(Definitions,Quantity,Status,bUsable);
}
void US1QuickSlotWidget::NativeConstruct()
{Super::NativeConstruct();ApplyUMGView(Definitions,Quantity,Status,bUsable);}
TSharedRef<SWidget> US1QuickSlotWidget::RebuildWidget()
{
 // Blueprint-authored layout owns geometry. Retain the native view only as a missing-asset fallback.
 if(WidgetTree&&WidgetTree->RootWidget)return Super::RebuildWidget();
 Icon=Cast<UTexture2D>(GetDefault<US1PotionPresentationSettings>()->Icon.TryLoad());
 Brush.SetResourceObject(Icon);Brush.ImageSize=FVector2D(40,40);
 auto Row=SNew(SHorizontalBox);
 for(int32 I=0;I<4;++I)
  Row->AddSlot().AutoWidth().Padding(3.f)
  [SNew(SBox).WidthOverride(100.f)
   [SNew(SBorder).Padding(8.f).BorderBackgroundColor(FLinearColor(0.04f,0.04f,0.04f,0.85f))
    [SNew(SVerticalBox)
     +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
     [SNew(STextBlock).Text(FText::AsNumber(I+1))]
     +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
     [SNew(SImage).Image(&Brush).Visibility_Lambda([this,I]{return Definitions.IsValidIndex(I)&&Definitions[I]==20001?EVisibility::Visible:EVisibility::Hidden;})
      .ColorAndOpacity_Lambda([this]{return bUsable&&Quantity>0?FLinearColor::White:FLinearColor(0.4f,0.4f,0.4f,1);})]
     +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
     [SNew(STextBlock).Text_Lambda([this,I]{return FText::FromString(Definitions.IsValidIndex(I)&&Definitions[I]==20001?FString::Printf(TEXT("%lld"),Quantity):TEXT(""));})]
     +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
     [SNew(STextBlock).Text_Lambda([this,I]{return FText::FromString(Definitions.IsValidIndex(I)&&Definitions[I]==20001?Status:FString());})]]]];
 return SNew(SVerticalBox)
  +SVerticalBox::Slot().FillHeight(1.f)
  +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,24)[Row];
}
void US1QuickSlotWidget::ReleaseSlateResources(bool bReleaseChildren)
{Super::ReleaseSlateResources(bReleaseChildren);Brush.SetResourceObject(nullptr);Icon=nullptr;}

int32 US1QuickSlotWidget::SlotAtPosition(const FVector2D& Position) const
{
 const UImage* Icons[4]={Icon1,Icon2,Icon3,Icon4};
 for(int32 I=0;I<4;++I)
  if(Icons[I]&&Icons[I]->GetParent()&&Icons[I]->GetParent()->GetCachedGeometry().IsUnderLocation(Position))return I;
 return INDEX_NONE;
}
FReply US1QuickSlotWidget::NativeOnMouseButtonDown(const FGeometry& Geometry,const FPointerEvent& Event)
{
 if(!IsDesignTime()&&Event.GetEffectingButton()==EKeys::RightMouseButton)
 {
  const int32 I=SlotAtPosition(Event.GetScreenSpacePosition());
  if(I!=INDEX_NONE)
  {if(auto* GI=GetGameInstance())GI->GetSubsystem<US1InventorySubsystem>()->ClearQuickSlot(I);return FReply::Handled();}
 }
 return Super::NativeOnMouseButtonDown(Geometry,Event);
}
bool US1QuickSlotWidget::NativeOnDrop(const FGeometry& Geometry,const FDragDropEvent& Event,UDragDropOperation* Operation)
{
 auto* Drag=Cast<US1InventoryDragOperation>(Operation);
 const int32 I=SlotAtPosition(Event.GetScreenSpacePosition());
 if(IsDesignTime()||!Drag||I==INDEX_NONE)return false;
 auto* GI=GetGameInstance();auto* Coordinator=GI?GI->GetSubsystem<US1InventorySubsystem>():nullptr;
 return Coordinator&&Coordinator->RegisterQuickSlotDrag(I,Drag->ItemId,Drag->SourceWindow.Get());
}
