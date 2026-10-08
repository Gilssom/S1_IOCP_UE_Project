#include "Game/S1DuskAnimInstance.h"

#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimationPoseData.h"
#include "AnimationRuntime.h"

namespace
{
class FS1DuskAnimProxy final : public FAnimInstanceProxy
{
public:
	explicit FS1DuskAnimProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}

protected:
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override
	{
		FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
		const US1DuskAnimInstance* Dusk = CastChecked<US1DuskAnimInstance>(InAnimInstance);
		if (AppliedRevision == Dusk->GetRequestRevision())
		{
			return;
		}

		const ES1DuskAnimationState NextState = Dusk->GetRequestedState();
		PreviousSequence = CurrentSequence;
		PreviousTime = CurrentTime;
		PreviousLooping = CurrentState == ES1DuskAnimationState::Idle ||
			CurrentState == ES1DuskAnimationState::Jog;
		BlendDuration = NextState == ES1DuskAnimationState::Attack ? Dusk->GetAttackEnterBlend() :
			CurrentState == ES1DuskAnimationState::Attack ? Dusk->GetAttackExitBlend() :
			NextState == ES1DuskAnimationState::Death ? Dusk->GetAttackEnterBlend() :
			Dusk->GetLocomotionBlend();
		BlendElapsed = 0.f;
		CurrentState = NextState;
		CurrentSequence = Dusk->GetSequence(NextState);
		CurrentTime = 0.f;
		AppliedRevision = Dusk->GetRequestRevision();
	}

	virtual void Update(float DeltaSeconds) override
	{
		const float Step = FMath::Max(0.f, DeltaSeconds);
		CurrentTime += Step;
		PreviousTime += Step;
		BlendElapsed += Step;
		if (BlendElapsed >= BlendDuration)
		{
			PreviousSequence = nullptr;
		}
	}

	virtual bool Evaluate(FPoseContext& Output) override
	{
		if (CurrentSequence == nullptr)
		{
			Output.ResetToRefPose();
			return true;
		}

		const bool bCurrentLooping = CurrentState == ES1DuskAnimationState::Idle ||
			CurrentState == ES1DuskAnimationState::Jog;
		if (PreviousSequence == nullptr || BlendDuration <= 0.f)
		{
			ExtractSequence(CurrentSequence, CurrentTime, bCurrentLooping, Output);
			return true;
		}

		FPoseContext PreviousPose(Output);
		FPoseContext CurrentPose(Output);
		ExtractSequence(PreviousSequence, PreviousTime, PreviousLooping, PreviousPose);
		ExtractSequence(CurrentSequence, CurrentTime, bCurrentLooping, CurrentPose);
		FAnimationPoseData PreviousData(PreviousPose);
		FAnimationPoseData CurrentData(CurrentPose);
		FAnimationPoseData OutputData(Output);
		const float PreviousWeight = 1.f - FMath::Clamp(BlendElapsed / BlendDuration, 0.f, 1.f);
		FAnimationRuntime::BlendTwoPosesTogether(PreviousData, CurrentData, PreviousWeight, OutputData);
		return true;
	}

private:
	static void ExtractSequence(UAnimSequence* Sequence, float Time, bool bLoop, FPoseContext& Output)
	{
		const float Length = Sequence->GetPlayLength();
		const double SampleTime = bLoop && Length > 0.f ?
			FMath::Fmod(Time, Length) : FMath::Clamp(Time, 0.f, Length);
		FAnimationPoseData PoseData(Output);
		Sequence->GetAnimationPose(PoseData, FAnimExtractContext(SampleTime, false, {}, bLoop));
	}

	UAnimSequence* CurrentSequence = nullptr;
	UAnimSequence* PreviousSequence = nullptr;
	ES1DuskAnimationState CurrentState = ES1DuskAnimationState::Idle;
	uint32 AppliedRevision = 0;
	float CurrentTime = 0.f;
	float PreviousTime = 0.f;
	float BlendDuration = 0.f;
	float BlendElapsed = 0.f;
	bool PreviousLooping = false;
};
}

void US1DuskAnimInstance::Configure(UAnimSequence* InIdle, UAnimSequence* InJog,
	UAnimSequence* InAttack, UAnimSequence* InDeath)
{
	IdleSequence = InIdle;
	JogSequence = InJog;
	AttackSequence = InAttack;
	DeathSequence = InDeath;
	++RequestRevision;
}

void US1DuskAnimInstance::SetLocomotion(bool bJog)
{
	if (RequestedState == ES1DuskAnimationState::Death)
	{
		return;
	}
	const ES1DuskAnimationState Next = bJog ? ES1DuskAnimationState::Jog : ES1DuskAnimationState::Idle;
	if (RequestedState != Next)
	{
		RequestedState = Next;
		++RequestRevision;
	}
}

void US1DuskAnimInstance::PlayAttack()
{
	if (RequestedState != ES1DuskAnimationState::Death)
	{
		RequestedState = ES1DuskAnimationState::Attack;
		++RequestRevision;
	}
}

void US1DuskAnimInstance::PlayDeath()
{
	if (RequestedState != ES1DuskAnimationState::Death)
	{
		RequestedState = ES1DuskAnimationState::Death;
		++RequestRevision;
	}
}

void US1DuskAnimInstance::SetBlendTimes(float InAttackEnter, float InAttackExit, float InLocomotion)
{
	AttackEnterBlend = FMath::Max(0.f, InAttackEnter);
	AttackExitBlend = FMath::Max(0.f, InAttackExit);
	LocomotionBlend = FMath::Max(0.f, InLocomotion);
}

UAnimSequence* US1DuskAnimInstance::GetSequence(ES1DuskAnimationState State) const
{
	switch (State)
	{
	case ES1DuskAnimationState::Jog: return JogSequence;
	case ES1DuskAnimationState::Attack: return AttackSequence;
	case ES1DuskAnimationState::Death: return DeathSequence;
	default: return IdleSequence;
	}
}

FAnimInstanceProxy* US1DuskAnimInstance::CreateAnimInstanceProxy()
{
	return new FS1DuskAnimProxy(this);
}

void US1DuskAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete InProxy;
}
