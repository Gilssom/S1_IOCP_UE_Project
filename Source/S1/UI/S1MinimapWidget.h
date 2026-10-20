#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "S1MinimapWidget.generated.h"
class UImage;
class UTextBlock;
class UTexture2D;

/** Passive north-up presentation. No capture actor, navigation or authoritative state. */
UCLASS()
class S1_API US1MinimapWidget : public UUserWidget
{
 GENERATED_BODY()
public:
 void SetPlayerView(const FVector& Position,float Yaw);
 static FVector2D ProjectPosition(const FVector& Position,const FVector2D& Origin,const FVector2D& HalfExtent);
protected:
 virtual void NativePreConstruct() override;
 virtual void NativeTick(const FGeometry& Geometry,float DeltaTime) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Minimap") TObjectPtr<UTexture2D> MapTexture;
 // Texture top=north(+X), right=east(+Y). XY centimeters, same world space as pawn.
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Minimap") FVector2D MapWorldOrigin=FVector2D::ZeroVector;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Minimap",meta=(ClampMin="1")) FVector2D MapWorldHalfExtent=FVector2D(10000,10000);
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> MapImage;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> PlayerMarker;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> PositionText;
 UPROPERTY(meta=(BindWidget)) TObjectPtr<UTextBlock> MapStatusText;
};
