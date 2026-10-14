#include "UI/S1EntityHealthWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void US1EntityHealthWidget::SetServerView(uint32 Current,uint32 Maximum,bool bDead,const FText& Name)
{
 Hp=Current; MaxHp=Maximum; bIsDead=bDead; DisplayName=Name;
 RefreshView();
}
float US1EntityHealthWidget::GetHealthFraction() const
{
 return MaxHp ? FMath::Clamp(static_cast<float>(Hp)/static_cast<float>(MaxHp),0.f,1.f) : 0.f;
}
void US1EntityHealthWidget::NativeConstruct()
{
 Super::NativeConstruct();
 SetIsFocusable(false);
 SetVisibility(ESlateVisibility::HitTestInvisible);
 RefreshView();
}
void US1EntityHealthWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(IsDesignTime())SetServerView(75,100,false,FText::FromString(TEXT("Preview")));
 else RefreshView();
}
void US1EntityHealthWidget::RefreshView()
{
 if(HealthBar)HealthBar->SetPercent(GetHealthFraction());
 if(NameText)NameText->SetText(DisplayName);
 if(HealthText)HealthText->SetText(FText::FromString(FString::Printf(TEXT("%u / %u%s"),Hp,MaxHp,bIsDead?TEXT(" [DEAD]"):TEXT(""))));
 // No styling/geometry changes: the Widget Blueprint designer owns those.
}
