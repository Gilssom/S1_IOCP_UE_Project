#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPath.h"
#include "S1CombatImpactSettings.generated.h"

UENUM()
enum class ES1CombatImpactFacing : uint8
{
	World,
	SourceToTarget,
	TargetFacing
};

USTRUCT()
struct FS1CombatImpactRule
{
	GENERATED_BODY()

	// Matches the wire CombatActionType value; no Unreal asset path is sent by the server.
	UPROPERTY(EditAnywhere, Category = "Impact", meta = (ClampMin = "1"))
	int32 ActionType = 0;

	UPROPERTY(EditAnywhere, Category = "Impact")
	FSoftObjectPath ParticleAsset;

	// Centimeters in the target actor's unscaled local space (capsule center origin).
	UPROPERTY(EditAnywhere, Category = "Impact")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Impact")
	ES1CombatImpactFacing Facing = ES1CombatImpactFacing::World;

	UPROPERTY(EditAnywhere, Category = "Impact")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Impact", meta = (ClampMin = "0.01"))
	float UniformScale = 1.f;

	// Safety cap even if an imported emitter loops or fails to auto-deactivate.
	UPROPERTY(EditAnywhere, Category = "Impact", meta = (ClampMin = "0.1"))
	float MaxLifeSeconds = 2.5f;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "S1 Combat Impact"))
class S1_API US1CombatImpactSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Impact")
	TArray<FS1CombatImpactRule> ImpactRules;
};
