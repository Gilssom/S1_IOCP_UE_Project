#pragma once
#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "S1InventoryDragOperation.generated.h"
class US1InventoryWidget;
// UI identifiers only, revalidated by the coordinator against current server state.
UCLASS()
class S1_API US1InventoryDragOperation : public UDragDropOperation
{
 GENERATED_BODY()
public:
 UPROPERTY() uint64 ItemId=0;
 UPROPERTY() uint32 SourceSlot=0;
 UPROPERTY() uint64 InventoryVersion=0;
 UPROPERTY() TWeakObjectPtr<US1InventoryWidget> SourceWindow;
};
