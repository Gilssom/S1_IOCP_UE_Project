#include "UI/S1MainHUDWidget.h"
#include "UI/S1EntityHealthComponent.h"
#include "Account/S1AccountSubsystem.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Game/S1MyPlayer.h"
#include "Loot/S1InventorySubsystem.h"
#include "Quest/S1QuestSubsystem.h"
#include "Quest/S1DialogueSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
void US1MainHUDWidget::NativeConstruct()
{
 Super::NativeConstruct();SetIsFocusable(false);
 SetLevelView(0);
 if(InventoryButton)InventoryButton->OnClicked.AddUniqueDynamic(this,&US1MainHUDWidget::InventoryClicked);
 if(QuestButton)QuestButton->OnClicked.AddUniqueDynamic(this,&US1MainHUDWidget::QuestClicked);
 SetCombatView(0,0,false,ES1KnightActionState::Ready,0);
}
void US1MainHUDWidget::NativeDestruct()
{
 if(InventoryButton)InventoryButton->OnClicked.RemoveDynamic(this,&US1MainHUDWidget::InventoryClicked);
 if(QuestButton)QuestButton->OnClicked.RemoveDynamic(this,&US1MainHUDWidget::QuestClicked);
 Super::NativeDestruct();
}
void US1MainHUDWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime())SetCombatView(75,100,false,ES1KnightActionState::Ready,0);
 if(IsDesignTime())SetLevelView(1);
}
void US1MainHUDWidget::SetLevelView(uint32 Level)
{
 if(LevelText)LevelText->SetText(FText::FromString(Level?FString::Printf(TEXT("Lv. %u"),Level):TEXT("Lv. —")));
}
void US1MainHUDWidget::SetCombatView(uint32 HP,uint32 Maximum,bool bDead,ES1KnightActionState Action,int32 Combo)
{
 const float Ratio=Maximum&&!bDead?FMath::Clamp(static_cast<float>(HP)/Maximum,0.f,1.f):0.f;
 if(HealthBar)HealthBar->SetPercent(Ratio);
 if(HealthText)HealthText->SetText(FText::FromString(!Maximum?TEXT("HP 동기화 대기"):
  FString::Printf(TEXT("HP  %u / %u%s"),FMath::Min(HP,Maximum),Maximum,bDead?TEXT(" · 사망"):TEXT(""))));
 const bool bPrimary=Action==ES1KnightActionState::PrimaryCombo;
 const bool bCharge=Action==ES1KnightActionState::ChargePresentation;
 if(PrimaryText)PrimaryText->SetText(FText::FromString(bPrimary?FString::Printf(TEXT("LMB · 연속 공격 %d / 4"),FMath::Clamp(Combo,1,4)):TEXT("LMB · 기본 공격")));
 if(ChargeText)ChargeText->SetText(FText::FromString(bCharge?TEXT("Q · 연속 베기 재생 중"):TEXT("Q · 연속 베기")));
 if(PrimaryFrame)PrimaryFrame->SetBrushColor(bPrimary?FLinearColor(.2f,.13f,.045f,.98f):FLinearColor(.02f,.03f,.05f,.92f));
 if(ChargeFrame)ChargeFrame->SetBrushColor(bCharge?FLinearColor(.2f,.13f,.045f,.98f):FLinearColor(.02f,.03f,.05f,.92f));
}
void US1MainHUDWidget::NativeTick(const FGeometry& Geometry,float DeltaTime)
{
 Super::NativeTick(Geometry,DeltaTime);
 const auto* GI=GetGameInstance();const auto* Account=GI?GI->GetSubsystem<US1AccountSubsystem>():nullptr;
 SetLevelView(Account?Account->GetActiveCharacterLevel():0);
 auto* Player=Cast<AS1MyPlayer>(GetOwningPlayerPawn());
 const auto* Health=Player?Player->FindComponentByClass<US1EntityHealthComponent>():nullptr;
 const auto* Combat=Player?Player->GetKnightCombatComponent():nullptr;
 SetCombatView(Health?Health->GetServerHP():0,Health?Health->GetServerMaxHP():0,Health&&Health->IsServerDead(),
  Combat?Combat->GetActionState():ES1KnightActionState::Ready,Combat?Combat->GetComboStep():0);
}
void US1MainHUDWidget::InventoryClicked()
{
 auto* World=GetWorld();auto* GI=GetGameInstance();if(!World||!GI)return;
 const auto* Dialogue=World->GetSubsystem<US1DialogueSubsystem>();
 const auto* Quest=GI->GetSubsystem<US1QuestSubsystem>();
 if((Dialogue&&Dialogue->IsModal())||(Quest&&Quest->IsJournalOpen()))return;
 if(auto* Player=Cast<AS1MyPlayer>(GetOwningPlayerPawn()))Player->StopForDialogue();
 GI->GetSubsystem<US1InventorySubsystem>()->ToggleInventory();
}
void US1MainHUDWidget::QuestClicked()
{
 auto* GI=GetGameInstance();if(!GI)return;
 if(GI->GetSubsystem<US1InventorySubsystem>()->IsInventoryOpen())return;
 GI->GetSubsystem<US1QuestSubsystem>()->ToggleJournal(true);
}
