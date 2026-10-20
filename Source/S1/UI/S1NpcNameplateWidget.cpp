#include "UI/S1NpcNameplateWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Actor.h"
#include "Quest/S1DialogueSubsystem.h"
#include "Engine/World.h"

void US1NpcNameplateWidget::SetNpcName(const FText& Name)
{
 NpcName=Name;RefreshView();
}
void US1NpcNameplateWidget::NativeConstruct()
{
 Super::NativeConstruct();SetIsFocusable(false);
 SetVisibility(ESlateVisibility::HitTestInvisible);RefreshView();
}
void US1NpcNameplateWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime()&&NameText)NameText->SetText(FText::FromString(TEXT("NPC 이름")));
 else RefreshView();
}
void US1NpcNameplateWidget::RefreshView()
{
 if(NameText)NameText->SetText(NpcName);
}
US1NpcNameplateComponent::US1NpcNameplateComponent()
{
 SetWidgetSpace(EWidgetSpace::Screen);SetDrawSize(FVector2D(300,36));
 SetPivot(FVector2D(.5,1));SetWindowFocusable(false);
 SetCollisionEnabled(ECollisionEnabled::NoCollision);SetCanEverAffectNavigation(false);
 SetVisibility(false);
}
void US1NpcNameplateComponent::InitWidget()
{
 Super::InitWidget();RefreshView();
}
void US1NpcNameplateComponent::SetNpcName(const FText& Name)
{
 NpcName=Name;RefreshView();
}
void US1NpcNameplateComponent::RefreshView()
{
 if(auto* View=Cast<US1NpcNameplateWidget>(GetUserWidgetObject()))View->SetNpcName(NpcName);
}
bool US1NpcNameplateComponent::ShouldShowAtDistance(bool bWasVisible,float DistanceSquared,float ShowCm,float HideCm)
{
 if(!FMath::IsFinite(DistanceSquared)||DistanceSquared<0.f)return false;
 const float Enter=FMath::Max(0.f,ShowCm);
 const float Exit=FMath::Max(Enter,HideCm);
 return bWasVisible ? DistanceSquared<=FMath::Square(Exit) : DistanceSquared<=FMath::Square(Enter);
}
void US1NpcNameplateComponent::UpdateForViewer(const AActor* Viewer)
{
 const AActor* Owner=GetOwner();
 bWithinRange=IsValid(Viewer)&&IsValid(Owner)&&
  ShouldShowAtDistance(bWithinRange,FVector::DistSquared(Viewer->GetActorLocation(),Owner->GetActorLocation()),ShowDistance,HideDistance);
 const auto* Dialogue=GetWorld()?GetWorld()->GetSubsystem<US1DialogueSubsystem>():nullptr;
 SetVisibility(bWithinRange&&!(Dialogue&&Dialogue->IsModal()));
}
