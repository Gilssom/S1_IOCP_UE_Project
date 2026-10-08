#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "S1InventoryWidget.generated.h"

class STextBlock;
class UTexture2D;
class SVerticalBox;
class UButton;
class UImage;
class UTextBlock;
class UScrollBox;
class US1InventoryStackWidget;
struct FS1InventoryEntryView {uint64 ItemId=0;uint32 Count=0,Slot=0;};
DECLARE_DELEGATE(FS1InventoryClose);
DECLARE_DELEGATE_OneParam(FS1InventoryUse,uint64);
DECLARE_DELEGATE_TwoParams(FS1QuickSlotRegister,int32,uint64);
DECLARE_DELEGATE_OneParam(FS1QuickSlotClear,int32);

/** Passive inventory view: no packets, sockets, travel or optimistic item grants. */
UCLASS()
class S1_API US1InventoryWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetPotionView(int64 Count, const FString& Status);
 void SetUseView(const TArray<FS1InventoryEntryView>& Entries,bool bAllowed,bool bRetry);
 FS1InventoryClose OnCloseRequested;
 FS1InventoryUse OnUseRequested;
 FS1QuickSlotRegister OnRegisterRequested;
 FS1QuickSlotClear OnClearRequested;
 TSubclassOf<US1InventoryStackWidget> GetStackEntryClass() const {return StackEntryClass;}
protected:
 virtual void NativeConstruct() override;
 virtual void NativePreConstruct() override;
 virtual void NativeDestruct() override;
 virtual TSharedRef<SWidget> RebuildWidget() override;
 virtual void ReleaseSlateResources(bool bReleaseChildren) override;
 virtual FReply NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event) override;
 UPROPERTY(EditDefaultsOnly,Category="Inventory|Designer") TSubclassOf<US1InventoryStackWidget> StackEntryClass;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> PotionIcon;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> QuantityLabel;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> StatusLabel;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> SelectionLabel;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> UseLabel;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UScrollBox> StackEntryList;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> UseButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> CloseButton;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Register1;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Register2;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Register3;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Register4;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Clear1;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Clear2;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Clear3;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Clear4;
private:
 friend class FS1InventoryWidgetBlueprintTest;
 void RefreshUMG();
 void ClearRows();
 void SelectEntry(uint64 ItemId);
 void RegisterSelected(int32 SlotIndex);
 void ClearSlot(int32 SlotIndex);
 UFUNCTION() void HandleUse();
 UFUNCTION() void HandleClose();
 UFUNCTION() void HandleRegister1();
 UFUNCTION() void HandleRegister2();
 UFUNCTION() void HandleRegister3();
 UFUNCTION() void HandleRegister4();
 UFUNCTION() void HandleClear1();
 UFUNCTION() void HandleClear2();
 UFUNCTION() void HandleClear3();
 UFUNCTION() void HandleClear4();
 UPROPERTY(Transient) TArray<TObjectPtr<US1InventoryStackWidget>> StackRows;
 UPROPERTY(Transient) TObjectPtr<UTexture2D> IconTexture;
 FSlateBrush IconBrush;
 TSharedPtr<STextBlock> QuantityText, StatusText;
 TSharedPtr<SVerticalBox> EntryList;
 TArray<FS1InventoryEntryView> DisplayEntries;
 uint64 SelectedItem=0;
 bool bUseAllowed=false,bRetryRequest=false;
 int64 DisplayCount = 0;
 FString DisplayStatus;
};
