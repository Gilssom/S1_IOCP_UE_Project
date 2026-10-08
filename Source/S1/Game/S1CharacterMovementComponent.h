#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "S1CharacterMovementComponent.generated.h"

UCLASS()
class S1_API US1CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	void SetNetworkAnimationData(const FVector& InVelocity)
	{
		Velocity = InVelocity;
		Acceleration =
			InVelocity.IsNearlyZero()
			? FVector::ZeroVector
			: InVelocity.GetSafeNormal() * GetMaxAcceleration();
	}
};
