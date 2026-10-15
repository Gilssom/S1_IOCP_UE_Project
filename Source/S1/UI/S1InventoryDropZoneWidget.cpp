#include "UI/S1InventoryDropZoneWidget.h"
#include "UI/S1InventoryDragOperation.h"
#include "UI/S1InventoryWidget.h"
#include "Loot/S1InventorySubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/Widget.h"
#include "Widgets/Layout/SBorder.h"
TSharedRef<SWidget> US1InventoryDropZoneWidget::RebuildWidget()
{return SNew(SBorder).BorderBackgroundColor(FLinearColor::Transparent);}
bool US1InventoryDropZoneWidget::IsOutsideArrangedPanel(const FGeometry& Geometry,const FVector2D& Position)
{return !Geometry.GetLocalSize().IsNearlyZero()&&!Geometry.IsUnderLocation(Position);}
bool US1InventoryDropZoneWidget::NativeOnDrop(const FGeometry&,const FDragDropEvent& Event,UDragDropOperation* Operation)
{
 auto* Drag=Cast<US1InventoryDragOperation>(Operation);if(!Drag||!Drag->SourceWindow.IsValid())return false;
 // Fail closed: a missing/unarranged panel is not proof of an outside drop.
 auto* Panel=Drag->SourceWindow->GetWidgetFromName(TEXT("InventoryPanel"));
 if(!Panel||!IsOutsideArrangedPanel(Panel->GetCachedGeometry(),Event.GetScreenSpacePosition()))return false;
 auto* GI=GetGameInstance();auto* Coordinator=GI?GI->GetSubsystem<US1InventorySubsystem>():nullptr;
 return Coordinator&&Coordinator->HandleDiscardDrop(Drag->ItemId,Drag->SourceSlot,Drag->InventoryVersion,Drag->SourceWindow.Get());
}
