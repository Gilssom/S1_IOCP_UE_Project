#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "S1PotionPresentationSettings.generated.h"

class UStaticMesh;
class UTexture2D;
class UNiagaraSystem;

/** Presentation only. These settings never determine server ownership or payment. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="S1 Potion Presentation"))
class S1_API US1PotionPresentationSettings : public UDeveloperSettings
{
 GENERATED_BODY()
public:
 US1PotionPresentationSettings();
 UPROPERTY(Config, EditAnywhere, Category="Potion", meta=(AllowedClasses="/Script/Engine.StaticMesh"))
 FSoftObjectPath WorldMesh;
 UPROPERTY(Config, EditAnywhere, Category="Potion", meta=(AllowedClasses="/Script/Engine.Texture2D"))
 FSoftObjectPath Icon;
 UPROPERTY(Config, EditAnywhere, Category="Potion", meta=(AllowedClasses="/Script/Niagara.NiagaraSystem"))
 FSoftObjectPath DropEffect;
 UPROPERTY(Config,EditAnywhere,Category="Healing",meta=(AllowedClasses="/Script/Niagara.NiagaraSystem"))
 FSoftObjectPath HealingEffect;
 UPROPERTY(Config,EditAnywhere,Category="Healing",meta=(ClampMin="0.01")) float HealingScale=1.f;
 UPROPERTY(Config,EditAnywhere,Category="Healing") FVector HealingFootOffset=FVector::ZeroVector;
 UPROPERTY(Config,EditAnywhere,Category="Healing",meta=(ClampMin="0.1",ClampMax="10")) float HealingEmitSeconds=1.5f;
 UPROPERTY(Config,EditAnywhere,Category="Healing",meta=(ClampMin="0.2",ClampMax="15")) float HealingMaxLifeSeconds=4.f;
 UPROPERTY(Config, EditAnywhere, Category="Potion") FString DisplayName = TEXT("회복 포션");
 UPROPERTY(Config, EditAnywhere, Category="Presentation", meta=(ClampMin="0.01"))
 float MeshScale = 0.4f;
 UPROPERTY(Config, EditAnywhere, Category="Presentation") float FloorOffset = 0.f;
 UPROPERTY(Config, EditAnywhere, Category="Presentation", meta=(ClampMin="0.01"))
 float EffectScale = 1.f;
private:
 // Exact default assets anchored in the native CDO for cooker discovery, not
 // folder-wide AlwaysCook rules. Runtime still resolves editable soft paths.
 UPROPERTY() TObjectPtr<UStaticMesh> CookDefaultMesh;
 UPROPERTY() TObjectPtr<UTexture2D> CookDefaultIcon;
 UPROPERTY() TObjectPtr<UNiagaraSystem> CookDefaultEffect;
 UPROPERTY() TObjectPtr<UNiagaraSystem> CookDefaultHealingEffect;
};
