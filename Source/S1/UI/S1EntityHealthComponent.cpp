#include "UI/S1EntityHealthComponent.h"
#include "UI/S1EntityHealthWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Quest/S1DialogueSubsystem.h"
#include "Engine/World.h"

US1EntityHealthComponent::US1EntityHealthComponent()
{
 SetWidgetSpace(EWidgetSpace::Screen);
 SetDrawSize(FVector2D(180.f,56.f));
 SetPivot(FVector2D(.5f,1.f));
 SetCollisionEnabled(ECollisionEnabled::NoCollision);
 SetGenerateOverlapEvents(false);
 SetWindowFocusable(false);
 SetVisibility(false);
 static ConstructorHelpers::FClassFinder<US1EntityHealthWidget> HealthBP(TEXT("/Game/S1/UI/WBP_S1EntityHealth"));
 HealthWidgetClass=HealthBP.Class;
}
void US1EntityHealthComponent::BeginPlay()
{
 // Observe dialogue close even while the screen-space component is hidden.
 SetTickMode(ETickMode::Enabled);
 // Hard default class reference ensures the passive WBP is reachable during cooking.
 if(HealthWidgetClass)SetWidgetClass(HealthWidgetClass);
 Super::BeginPlay();
 InitWidget();
 RefreshView();
}
void US1EntityHealthComponent::InitWidget()
{
 Super::InitWidget();
 RefreshView();
}
void US1EntityHealthComponent::SetServerView(uint32 Current,uint32 Maximum,bool bIsDead,const FText& Name)
{
 Hp=Current;MaxHp=Maximum;bDead=bIsDead;DisplayName=Name;
 RefreshView();
}
void US1EntityHealthComponent::RefreshView()
{
 const auto* Dialogue=GetWorld()?GetWorld()->GetSubsystem<US1DialogueSubsystem>():nullptr;
 bLastDialogueModal=Dialogue&&Dialogue->IsModal();
 SetVisibility(!bLastDialogueModal && !bDialogueSuppressed && bPresentationEnabled && MaxHp>0 && !(bHideOnDeath && bDead));
 if(auto* View=Cast<US1EntityHealthWidget>(GetUserWidgetObject()))View->SetServerView(Hp,MaxHp,bDead,DisplayName);
}
void US1EntityHealthComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
 Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
 const auto* Dialogue=GetWorld()?GetWorld()->GetSubsystem<US1DialogueSubsystem>():nullptr;
 if(bLastDialogueModal!=bool(Dialogue&&Dialogue->IsModal()))RefreshView();
}
void US1EntityHealthComponent::SetPresentationEnabled(bool bEnabled)
{
 if(bPresentationEnabled==bEnabled)return;
 bPresentationEnabled=bEnabled;
 RefreshView();
}
void US1EntityHealthComponent::SetDialogueSuppressed(bool bSuppressed)
{
 if(bDialogueSuppressed==bSuppressed)return;
 bDialogueSuppressed=bSuppressed;
 RefreshView();
}
