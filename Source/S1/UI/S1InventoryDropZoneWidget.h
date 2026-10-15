#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1InventoryDropZoneWidget.generated.h"
class UWidget;
/** Lowest-z inventory-only drop receiver, below quick-slots and every other UI. */
UCLASS()
class S1_API US1InventoryDropZoneWidget : public UUserWidget
{
 GENERATED_BODY()
 friend class FS1InventoryDiscardTest;
 static bool IsOutsideArrangedPanel(const FGeometry& Geometry,const FVector2D& Position);
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual bool NativeOnDrop(const FGeometry&,const FDragDropEvent&,UDragDropOperation*) override;
};
