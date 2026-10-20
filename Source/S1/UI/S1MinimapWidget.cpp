#include "UI/S1MinimapWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/Pawn.h"
#include "Engine/Texture2D.h"

FVector2D US1MinimapWidget::ProjectPosition(const FVector& Position,const FVector2D& Origin,const FVector2D& HalfExtent)
{
 return FVector2D(FMath::Clamp(.5+(Position.Y-Origin.Y)/(2.*FMath::Max(1.,HalfExtent.Y)),.04,.96),
  FMath::Clamp(.5-(Position.X-Origin.X)/(2.*FMath::Max(1.,HalfExtent.X)),.04,.96));
}
void US1MinimapWidget::NativePreConstruct()
{
 Super::NativePreConstruct();
 if(MapImage){MapImage->SetBrushFromTexture(MapTexture,false);MapImage->SetVisibility(MapTexture?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);}
 if(MapStatusText)MapStatusText->SetText(MapTexture?FText::GetEmpty():FText::FromString(TEXT("지도 텍스처 미설정")));
 SetPlayerView(FVector::ZeroVector,0);
}
void US1MinimapWidget::SetPlayerView(const FVector& Position,float Yaw)
{
 if(PositionText)PositionText->SetText(FText::FromString(FString::Printf(TEXT("X %.0fm  Y %.0fm"),Position.X/100.,Position.Y/100.)));
 if(PlayerMarker)
 {
  // With no geographic image, remain an honest local compass rather than inventing terrain.
  const FVector2D UV=MapTexture?ProjectPosition(Position,MapWorldOrigin,MapWorldHalfExtent):FVector2D(.5,.5);
  if(auto* MarkerSlot=Cast<UCanvasPanelSlot>(PlayerMarker->Slot))
  {FAnchors Anchor;Anchor.Minimum=Anchor.Maximum=UV;MarkerSlot->SetAnchors(Anchor);MarkerSlot->SetAlignment(FVector2D(.5,.5));}
  PlayerMarker->SetRenderTransformAngle(FMath::IsFinite(Yaw)?FMath::UnwindDegrees(Yaw):0.f);
 }
}
void US1MinimapWidget::NativeTick(const FGeometry& Geometry,float DeltaTime)
{
 Super::NativeTick(Geometry,DeltaTime);
 if(const auto* Pawn=GetOwningPlayerPawn())SetPlayerView(Pawn->GetActorLocation(),Pawn->GetActorRotation().Yaw);
}
