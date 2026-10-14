#include "UI/S1InventoryWidget.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "Engine/Texture2D.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "InputCoreTypes.h"
#include "UI/S1InventoryStackWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/UniformGridPanel.h"
#include "UI/S1InventorySlotWidget.h"

FReply US1InventoryWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
 if(!IsDesignTime()&&bRetryRequest&&Event.GetKey()==EKeys::R)
 {HandleUse();return FReply::Handled();}
 if(!IsDesignTime()&&(Event.GetKey()==EKeys::I||Event.GetKey()==EKeys::Escape))
 {HandleClose();return FReply::Handled();}
 return Super::NativeOnPreviewKeyDown(Geometry,Event);
}

void US1InventoryWidget::NativeConstruct()
{
 Super::NativeConstruct();
 if(InventoryGrid)SetVisibility(ESlateVisibility::SelfHitTestInvisible);
 if(UseButton)UseButton->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleUse);
 if(CloseButton)CloseButton->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleClose);
 if(Register1)Register1->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleRegister1);
 if(Register2)Register2->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleRegister2);
 if(Register3)Register3->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleRegister3);
 if(Register4)Register4->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleRegister4);
 if(Clear1)Clear1->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleClear1);
 if(Clear2)Clear2->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleClear2);
 if(Clear3)Clear3->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleClear3);
 if(Clear4)Clear4->OnClicked.AddUniqueDynamic(this,&US1InventoryWidget::HandleClear4);
 RefreshUMG();
}
void US1InventoryWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime())
 {
  // Preview instance only: no subsystem, requests, HP or saved inventory changes.
  DisplayCount=7;DisplayStatus=TEXT("디자인 미리보기 (서버 데이터 아님)");
  DisplayEntries={{1,2,0},{2,5,1}};SelectedItem=1;bUseAllowed=true;bRetryRequest=false;
 }
 RefreshUMG();
}
void US1InventoryWidget::ClearRows()
{
 for(const auto& Cell:GridSlots)if(Cell){Cell->OnSelected.Unbind();Cell->OnUseRequested.Unbind();}
 GridSlots.Reset();if(InventoryGrid)InventoryGrid->ClearChildren();
 for(const auto& Row:StackRows)if(Row)Row->OnSelected.Unbind();
 StackRows.Reset();if(StackEntryList)StackEntryList->ClearChildren();
}
void US1InventoryWidget::NativeDestruct()
{ClearRows();Super::NativeDestruct();}
void US1InventoryWidget::SelectEntry(uint64 ItemId)
{
 if(IsDesignTime()||bRetryRequest||!DisplayEntries.ContainsByPredicate([ItemId](const auto& E){return E.ItemId==ItemId;}))return;
 SelectedItem=ItemId;RefreshUMG();
}
void US1InventoryWidget::RegisterSelected(int32 SlotIndex)
{if(!IsDesignTime()&&bUseAllowed&&!bRetryRequest&&SelectedItem)OnRegisterRequested.ExecuteIfBound(SlotIndex,SelectedItem);}
void US1InventoryWidget::ClearSlot(int32 SlotIndex)
{if(!IsDesignTime())OnClearRequested.ExecuteIfBound(SlotIndex);}
void US1InventoryWidget::HandleUse()
{if(!IsDesignTime()&&bUseAllowed&&(SelectedItem||bRetryRequest))OnUseRequested.ExecuteIfBound(SelectedItem);}
void US1InventoryWidget::HandleClose(){if(!IsDesignTime())OnCloseRequested.ExecuteIfBound();}
void US1InventoryWidget::HandleRegister1(){RegisterSelected(0);}
void US1InventoryWidget::HandleRegister2(){RegisterSelected(1);}
void US1InventoryWidget::HandleRegister3(){RegisterSelected(2);}
void US1InventoryWidget::HandleRegister4(){RegisterSelected(3);}
void US1InventoryWidget::HandleClear1(){ClearSlot(0);}
void US1InventoryWidget::HandleClear2(){ClearSlot(1);}
void US1InventoryWidget::HandleClear3(){ClearSlot(2);}
void US1InventoryWidget::HandleClear4(){ClearSlot(3);}

void US1InventoryWidget::RefreshUMG()
{
 if(QuantityLabel)QuantityLabel->SetText(FText::FromString(FString::Printf(TEXT("%s x%lld"),*GetDefault<US1PotionPresentationSettings>()->DisplayName,DisplayCount)));
 if(StatusLabel)StatusLabel->SetText(FText::FromString(DisplayStatus));
 if(PotionIcon)
 {
  if(!IconTexture)IconTexture=Cast<UTexture2D>(GetDefault<US1PotionPresentationSettings>()->Icon.TryLoad());
  if(PotionIcon->GetBrush().GetResourceObject()!=IconTexture)PotionIcon->SetBrushFromTexture(IconTexture,false);
 }
 const auto* Selected=DisplayEntries.FindByPredicate([this](const auto& E){return E.ItemId==SelectedItem;});
 if(SelectionLabel)SelectionLabel->SetText(FText::FromString(bRetryRequest?TEXT("미확정: R키로 원래 사용 요청 재확인"):
  Selected?FString::Printf(TEXT("선택: 슬롯 %u · x%u"),Selected->Slot+1,Selected->Count):TEXT("선택 없음")));
 if(UseLabel)UseLabel->SetText(FText::FromString(bRetryRequest?TEXT("같은 사용 요청 재확인"):TEXT("선택 포션 사용 (+50 HP)")));
 if(UseButton)UseButton->SetIsEnabled(bUseAllowed&&(SelectedItem!=0||bRetryRequest));
 UButton* Registers[4]={Register1,Register2,Register3,Register4};
 for(auto* Button:Registers)if(Button)Button->SetIsEnabled(bUseAllowed&&!bRetryRequest&&SelectedItem!=0);
 if(InventoryGrid){RefreshGrid();return;}
 if(!StackEntryList||!StackEntryClass)return;
 bool bRebuild=StackRows.Num()!=DisplayEntries.Num();
 if(!bRebuild)for(int32 I=0;I<StackRows.Num();++I)
  if(!StackRows[I]||StackRows[I]->GetItemId()!=DisplayEntries[I].ItemId){bRebuild=true;break;}
 if(bRebuild)
 {
  ClearRows();
  for(const auto& Entry:DisplayEntries)
  {
   auto* Row=CreateWidget<US1InventoryStackWidget>(this,StackEntryClass);
   if(!Row)continue;
   Row->SetEntry(Entry,Entry.ItemId==SelectedItem,!bRetryRequest);
   Row->OnSelected.BindUObject(this,&US1InventoryWidget::SelectEntry);
   StackRows.Add(Row);StackEntryList->AddChild(Row);
  }
 }
 for(int32 I=0;I<FMath::Min(StackRows.Num(),DisplayEntries.Num());++I)
  StackRows[I]->SetEntry(DisplayEntries[I],DisplayEntries[I].ItemId==SelectedItem,!bRetryRequest);
}

void US1InventoryWidget::RefreshGrid()
{
 if(!InventoryGrid||!GridSlotClass)return;
 if(GridSlots.Num()!=80)
 {
  ClearRows();
  for(int32 I=0;I<80;++I)
  {
   auto* Cell=CreateWidget<US1InventorySlotWidget>(this,GridSlotClass);
   if(!Cell)continue;
   Cell->OnSelected.BindUObject(this,&US1InventoryWidget::SelectEntry);
   Cell->OnUseRequested.BindUObject(this,&US1InventoryWidget::UseEntry);
   GridSlots.Add(Cell);InventoryGrid->AddChildToUniformGrid(Cell,I/8,I%8);
  }
 }
 for(int32 I=0;I<GridSlots.Num();++I)
 {
  const auto* Entry=DisplayEntries.FindByPredicate([I](const auto& E){return E.Slot==static_cast<uint32>(I);});
  GridSlots[I]->SetSlot(Entry?*Entry:FS1InventoryEntryView{0,0,static_cast<uint32>(I)},bUseAllowed&&!bRetryRequest,I>=20,this);
 }
}
void US1InventoryWidget::UseEntry(uint64 ItemId)
{
 if(IsDesignTime()||!bUseAllowed||bRetryRequest)return;
 if(!DisplayEntries.ContainsByPredicate([ItemId](const auto& E){return E.ItemId==ItemId&&E.Count>0;}))return;
 SelectedItem=ItemId;OnUseRequested.ExecuteIfBound(ItemId);
}

FReply US1InventoryWidget::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
 if(Event.GetKey()==EKeys::I||Event.GetKey()==EKeys::Escape)
 {OnCloseRequested.ExecuteIfBound();return FReply::Handled();}
 return Super::NativeOnKeyDown(Geometry,Event);
}

TSharedRef<SWidget> US1InventoryWidget::RebuildWidget()
{
 if(WidgetTree&&WidgetTree->RootWidget)return Super::RebuildWidget();
 const auto* Settings = GetDefault<US1PotionPresentationSettings>();
 IconTexture = Cast<UTexture2D>(Settings->Icon.TryLoad());
 IconBrush.SetResourceObject(IconTexture);
 IconBrush.ImageSize = FVector2D(64.f,64.f);
 auto Registration = SNew(SHorizontalBox);
 for(int32 I=0;I<4;++I)
  Registration->AddSlot().FillWidth(1.f).Padding(2.f)
  [SNew(SVerticalBox)
   +SVerticalBox::Slot().AutoHeight()
   [SNew(SButton).IsEnabled_Lambda([this]{return bUseAllowed&&!bRetryRequest&&SelectedItem!=0;})
    .OnClicked_Lambda([this,I]{OnRegisterRequested.ExecuteIfBound(I,SelectedItem);return FReply::Handled();})
    [SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%d 등록"),I+1)))]]
   +SVerticalBox::Slot().AutoHeight()
   [SNew(SButton).OnClicked_Lambda([this,I]{OnClearRequested.ExecuteIfBound(I);return FReply::Handled();})
    [SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%d 해제"),I+1)))]]];
 auto Result = SNew(SBox).WidthOverride(400.f)
 [SNew(SBorder).Padding(16.f)
  [SNew(SVerticalBox)
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)
   [SNew(STextBlock).Text(FText::FromString(TEXT("인벤토리")))]
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)
   [SNew(SHorizontalBox)
    +SHorizontalBox::Slot().AutoWidth()[SNew(SImage).Image(&IconBrush)]
    +SHorizontalBox::Slot().FillWidth(1.f).Padding(12.f)
    [SAssignNew(QuantityText,STextBlock)]]
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)[SAssignNew(StatusText,STextBlock)]
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)[SAssignNew(EntryList,SVerticalBox)]
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)[Registration]
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)
   [SNew(SButton).IsEnabled_Lambda([this]{return bUseAllowed&&(SelectedItem!=0||bRetryRequest);})
    .OnClicked_Lambda([this]{OnUseRequested.ExecuteIfBound(SelectedItem);return FReply::Handled();})
    [SNew(STextBlock).Text_Lambda([this]{return FText::FromString(bRetryRequest?TEXT("같은 사용 요청 재확인"):TEXT("선택 포션 사용 (+50 HP)"));})]]
   +SVerticalBox::Slot().AutoHeight().Padding(4.f)
   [SNew(SButton).OnClicked_Lambda([this]() {OnCloseRequested.ExecuteIfBound(); return FReply::Handled();})
    [SNew(STextBlock).Text(FText::FromString(TEXT("닫기")))]]]];
 SetPotionView(DisplayCount,DisplayStatus);
 SetUseView(DisplayEntries,bUseAllowed,bRetryRequest);
 return Result;
}

void US1InventoryWidget::SetPotionView(int64 Count,const FString& Status)
{
 DisplayCount = FMath::Max<int64>(0,Count);
 DisplayStatus = Status;
 if(QuantityText) QuantityText->SetText(FText::FromString(FString::Printf(TEXT("%s x%lld"),
  *GetDefault<US1PotionPresentationSettings>()->DisplayName,DisplayCount)));
 if(StatusText) StatusText->SetText(FText::FromString(Status));
 RefreshUMG();
}

void US1InventoryWidget::ReleaseSlateResources(bool bReleaseChildren)
{
 Super::ReleaseSlateResources(bReleaseChildren);
 QuantityText.Reset(); StatusText.Reset();EntryList.Reset();
 IconBrush.SetResourceObject(nullptr); IconTexture = nullptr;
}

void US1InventoryWidget::SetUseView(const TArray<FS1InventoryEntryView>& Entries,bool bAllowed,bool bRetry)
{
 DisplayEntries=Entries;bUseAllowed=bAllowed;bRetryRequest=bRetry;
 if(!DisplayEntries.ContainsByPredicate([this](const auto& Entry){return Entry.ItemId==SelectedItem;}))SelectedItem=0;
 RefreshUMG();
 if(!EntryList)return;EntryList->ClearChildren();
 for(const auto& Entry:DisplayEntries)
  EntryList->AddSlot().AutoHeight().Padding(2.f)
  [SNew(SButton).IsEnabled_Lambda([this]{return !bRetryRequest;})
   .OnClicked_Lambda([this,Id=Entry.ItemId]{SelectedItem=Id;return FReply::Handled();})
   [SNew(STextBlock).Text_Lambda([this,Entry]{return FText::FromString(FString::Printf(TEXT("%s 슬롯 %u: 회복 포션 x%u"),
    SelectedItem==Entry.ItemId?TEXT("▶"):TEXT("  "),Entry.Slot+1,Entry.Count));})]];
}
