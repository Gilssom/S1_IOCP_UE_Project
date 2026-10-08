#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "S1KnightGameplayAbilities.generated.h"

UCLASS()
class S1_API US1KnightPrimaryComboAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	US1KnightPrimaryComboAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class S1_API US1KnightChargeAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	US1KnightChargeAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
