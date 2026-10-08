#include "Abilities/S1KnightGameplayAbilities.h"

#include "Combat/S1KnightCombatComponent.h"
#include "Game/S1Player.h"
#include "GameFramework/Controller.h"

US1KnightPrimaryComboAbility::US1KnightPrimaryComboAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void US1KnightPrimaryComboAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AS1Player* Player = ActorInfo != nullptr ? Cast<AS1Player>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (US1KnightCombatComponent* Combat = Player != nullptr ? Player->GetKnightCombatComponent() : nullptr)
	{
		const float CameraYaw = Player->GetController() != nullptr
			? Player->GetController()->GetControlRotation().Yaw
			: Player->GetActorRotation().Yaw;
		Combat->RequestPrimaryAttack(CameraYaw);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

US1KnightChargeAbility::US1KnightChargeAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void US1KnightChargeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	AS1Player* Player = ActorInfo != nullptr ? Cast<AS1Player>(ActorInfo->AvatarActor.Get()) : nullptr;
	if (US1KnightCombatComponent* Combat = Player != nullptr ? Player->GetKnightCombatComponent() : nullptr)
	{
		Combat->RequestChargePresentation(Player->GetActorRotation().Yaw);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
