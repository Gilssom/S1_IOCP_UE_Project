#pragma once

#include "Animation/AnimInstance.h"
#include "S1DuskAnimInstance.generated.h"

class UAnimSequence;

// Presentation only. The IOCP server remains authoritative for actions and damage.
UENUM()
enum class ES1DuskAnimationState : uint8
{
	Idle,
	Jog,
	Attack,
	Death
};

UCLASS(Transient, Blueprintable)
class S1_API US1DuskAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void Configure(UAnimSequence* InIdle, UAnimSequence* InJog,
		UAnimSequence* InAttack, UAnimSequence* InDeath);
	void SetLocomotion(bool bJog);
	void PlayAttack();
	void PlayDeath();
	void SetBlendTimes(float InAttackEnter, float InAttackExit, float InLocomotion);

	UAnimSequence* GetSequence(ES1DuskAnimationState State) const;
	ES1DuskAnimationState GetRequestedState() const { return RequestedState; }
	uint32 GetRequestRevision() const { return RequestRevision; }
	float GetAttackEnterBlend() const { return AttackEnterBlend; }
	float GetAttackExitBlend() const { return AttackExitBlend; }
	float GetLocomotionBlend() const { return LocomotionBlend; }

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> IdleSequence;
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> JogSequence;
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> AttackSequence;
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> DeathSequence;

	ES1DuskAnimationState RequestedState = ES1DuskAnimationState::Idle;
	uint32 RequestRevision = 1;
	float AttackEnterBlend = 0.08f;
	float AttackExitBlend = 0.15f;
	float LocomotionBlend = 0.15f;
};
