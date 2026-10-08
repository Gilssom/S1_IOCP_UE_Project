#include "Combat/S1KnightCombatComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

US1KnightCombatComponent::US1KnightCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ComboA(TEXT("/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_A_slow_Montage.Primary_Melee_A_slow_Montage"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ComboB(TEXT("/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_B_slow_Montage.Primary_Melee_B_slow_Montage"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ComboC(TEXT("/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_C_slow_Montage.Primary_Melee_C_slow_Montage"));
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ComboD(TEXT("/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Primary_Melee_D_slow_Montage.Primary_Melee_D_slow_Montage"));
	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> Charge(TEXT("/Game/ParagonTerra/Characters/Heroes/Terra/Animations/Q_swing_B.Q_swing_B"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> Trail(TEXT("/Game/ParagonTerra/FX/Particles/Terra/Abilities/Primary/FX/P_Terra_AnimTrail.P_Terra_AnimTrail"));

	PrimaryComboMontages = { ComboA.Object, ComboB.Object, ComboC.Object, ComboD.Object };
	PrimaryTrailWindows = {
		FVector2D(0.23f, 0.73f),
		FVector2D(0.17f, 0.72f),
		FVector2D(0.15f, 0.65f),
		FVector2D(0.27f, 0.78f)
	};
	PrimaryTrailParticle = Trail.Object;
	ChargeAnimation = Charge.Object;
}

bool US1KnightCombatComponent::CanStartGroundAction() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	return Character != nullptr && Character->GetCharacterMovement() != nullptr
		&& Character->GetCharacterMovement()->IsMovingOnGround();
}

bool US1KnightCombatComponent::RequestPrimaryAttack(float FacingYaw)
{
	if (!CanStartGroundAction())
	{
		return false;
	}

	if (ActionState == ES1KnightActionState::Ready)
	{
		StartComboStep(0, FacingYaw, true);
		return ActionState == ES1KnightActionState::PrimaryCombo;
	}

	if (ActionState == ES1KnightActionState::PrimaryCombo &&
		!bNextComboBuffered)
	{
		bNextComboBuffered = true;
		BufferedFacingYaw = FacingYaw;
		UE_LOG(LogTemp, Log,
			TEXT("Knight combo input buffered. Actor[%s], CurrentStep[%d], AfterBranch[%s]"),
			*GetNameSafe(GetOwner()), ComboStep + 1,
			bComboBranchResolved ? TEXT("true") : TEXT("false"));
		return true;
	}

	return false;
}

void US1KnightCombatComponent::StartComboStep(
	int32 NewComboStep,
	float FacingYaw,
	bool bNotifyServer)
{
	if (!PrimaryComboMontages.IsValidIndex(NewComboStep) || PrimaryComboMontages[NewComboStep] == nullptr)
	{
		FinishCurrentAction();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInstance = Character != nullptr && Character->GetMesh() != nullptr
		? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance == nullptr)
	{
		FinishCurrentAction();
		return;
	}

	ClearActionTimers();
	StopPrimaryTrail();
	LockMovementFacing();
	ActionState = ES1KnightActionState::PrimaryCombo;
	ComboStep = NewComboStep;
	bNextComboBuffered = false;
	bComboBranchResolved = false;
	BufferedFacingYaw = FacingYaw;
	Character->SetActorRotation(FRotator(0.0f, FacingYaw, 0.0f));

	CurrentActionDuration = AnimInstance->Montage_Play(PrimaryComboMontages[ComboStep]);
	if (CurrentActionDuration <= 0.0f)
	{
		FinishCurrentAction();
		return;
	}
	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &US1KnightCombatComponent::OnPrimaryMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, PrimaryComboMontages[ComboStep]);
	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = SavedMaxWalkSpeed * AttackMovementSpeedMultiplier;
	}
	SchedulePrimaryTrail();

	FTimerManager& Timers = GetWorld()->GetTimerManager();
	Timers.SetTimer(ComboBranchTimer, this, &US1KnightCombatComponent::ResolveComboBranch,
		FMath::Max(0.02f, CurrentActionDuration * ComboBranchRatio), false);

	UE_LOG(LogTemp, Log, TEXT("Knight combo presentation started. Actor[%s], Step[%d], FacingYaw[%.2f]"),
		*GetNameSafe(Character), ComboStep + 1, FacingYaw);

	if (bNotifyServer)
	{
		OnActionStarted.Broadcast(
			ES1KnightCombatAction::PrimaryCombo,
			ComboStep + 1,
			FacingYaw);
	}
}

void US1KnightCombatComponent::ResolveComboBranch()
{
	bComboBranchResolved = true;
	if (ActionState != ES1KnightActionState::PrimaryCombo)
	{
		return;
	}

	if (bNextComboBuffered)
	{
		StartBufferedNextStep();
		return;
	}

	const float RemainingDuration = FMath::Max(0.01f, CurrentActionDuration * (1.0f - ComboBranchRatio));
	GetWorld()->GetTimerManager().SetTimer(ActionEndTimer, this,
		&US1KnightCombatComponent::ResolveComboEnd, RemainingDuration, false);
}

void US1KnightCombatComponent::ResolveComboEnd()
{
	if (ActionState == ES1KnightActionState::PrimaryCombo && bNextComboBuffered)
	{
		StartBufferedNextStep();
		return;
	}
	FinishCurrentAction();
}

void US1KnightCombatComponent::StartBufferedNextStep()
{
	const int32 NextStep = (ComboStep + 1) % PrimaryComboMontages.Num();
	float NextFacingYaw = BufferedFacingYaw;
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character != nullptr && Character->IsLocallyControlled() &&
		Character->GetController() != nullptr)
	{
		NextFacingYaw = Character->GetController()->GetControlRotation().Yaw;
	}
	StartComboStep(NextStep, NextFacingYaw, true);
}

bool US1KnightCombatComponent::RequestChargePresentation(float FacingYaw)
{
	return StartChargePresentation(FacingYaw, true);
}

bool US1KnightCombatComponent::StartChargePresentation(float FacingYaw, bool bNotifyServer)
{
	if (ActionState != ES1KnightActionState::Ready || !CanStartGroundAction() || ChargeAnimation == nullptr)
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UAnimInstance* AnimInstance = Character != nullptr && Character->GetMesh() != nullptr
		? Character->GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance == nullptr)
	{
		return false;
	}

	Character->SetActorRotation(FRotator(0.0f, FacingYaw, 0.0f));
	LockMovementFacing();
	// ChargeAnimation accepts both sequences and authored montages. The dynamic
	// montage API rejects a montage asset, so play authored montage notifies directly.
	UAnimMontage* PlayedMontage = Cast<UAnimMontage>(ChargeAnimation);
	float PlayedDuration = 0.0f;
	if (PlayedMontage != nullptr)
	{
		PlayedDuration = AnimInstance->Montage_Play(
			PlayedMontage, 1.0f, EMontagePlayReturnType::Duration);
	}
	else
	{
		PlayedMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
			ChargeAnimation, TEXT("UpperBody"), 0.10f, 0.25f, 1.0f, 1, 0.0f);
		if (PlayedMontage != nullptr)
		{
			PlayedDuration = PlayedMontage->GetPlayLength();
		}
	}
	if (PlayedMontage == nullptr || PlayedDuration <= 0.0f)
	{
		RestoreMovementFacing();
		UE_LOG(LogTemp, Warning, TEXT("Knight Q playback failed. Actor[%s], Animation[%s]"),
			*GetNameSafe(Character), *GetPathNameSafe(ChargeAnimation));
		return false;
	}

	ActionState = ES1KnightActionState::ChargePresentation;
	Character->StopJumping();
	CurrentActionDuration = PlayedDuration;
	GetWorld()->GetTimerManager().SetTimer(ActionEndTimer, this,
		&US1KnightCombatComponent::FinishCurrentAction, CurrentActionDuration, false);

	UE_LOG(LogTemp, Log,
		TEXT("Knight Q presentation started without displacement. Actor[%s], FacingYaw[%.2f], Animation[%s], Duration[%.3f], Remote[%d]"),
		*GetNameSafe(Character), FacingYaw, *GetPathNameSafe(ChargeAnimation),
		CurrentActionDuration, !bNotifyServer);
	if (bNotifyServer)
	{
		OnActionStarted.Broadcast(
			ES1KnightCombatAction::Charge,
			0,
			FacingYaw);
	}
	return true;
}

bool US1KnightCombatComponent::PlayReplicatedAction(
	ES1KnightCombatAction Action,
	int32 ReplicatedComboStep,
	uint32 Sequence,
	float FacingYaw)
{
	if (Sequence == 0 || Sequence <= LastReplicatedActionSequence)
	{
		return false;
	}

	LastReplicatedActionSequence = Sequence;
	FinishCurrentAction();

	switch (Action)
	{
	case ES1KnightCombatAction::PrimaryCombo:
		if (ReplicatedComboStep < 1 || ReplicatedComboStep > PrimaryComboMontages.Num())
		{
			return false;
		}
		StartComboStep(ReplicatedComboStep - 1, FacingYaw, false);
		return ActionState == ES1KnightActionState::PrimaryCombo;

	case ES1KnightCombatAction::Charge:
		return ReplicatedComboStep == 0
			&& StartChargePresentation(FacingYaw, false);
	}

	return false;
}

void US1KnightCombatComponent::FinishCurrentAction()
{
	ClearActionTimers();
	StopPrimaryTrail();
	RestoreMovementFacing();
	ActionState = ES1KnightActionState::Ready;
	ComboStep = INDEX_NONE;
	CurrentPredictionSequence = 0;
	bNextComboBuffered = false;
	bComboBranchResolved = false;
	CurrentActionDuration = 0.0f;
}

void US1KnightCombatComponent::SchedulePrimaryTrail()
{
	if (GetNetMode() == NM_DedicatedServer || PrimaryTrailParticle == nullptr ||
		!PrimaryTrailWindows.IsValidIndex(ComboStep) || GetWorld() == nullptr)
	{
		return;
	}

	const FVector2D Window = PrimaryTrailWindows[ComboStep];
	if (Window.X < 0.0f || Window.Y <= Window.X || Window.Y > CurrentActionDuration)
	{
		UE_LOG(LogTemp, Warning, TEXT("Knight trail window invalid. Step[%d], Start[%.3f], End[%.3f], Duration[%.3f]"),
			ComboStep + 1, Window.X, Window.Y, CurrentActionDuration);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(PrimaryTrailStartTimer, this,
		&US1KnightCombatComponent::BeginPrimaryTrail, Window.X, false);
	GetWorld()->GetTimerManager().SetTimer(PrimaryTrailEndTimer, this,
		&US1KnightCombatComponent::EndPrimaryTrail, Window.Y, false);
}

void US1KnightCombatComponent::BeginPrimaryTrail()
{
	if (ActionState != ES1KnightActionState::PrimaryCombo ||
		!PrimaryComboMontages.IsValidIndex(ComboStep))
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* Mesh = Character != nullptr ? Character->GetMesh() : nullptr;
	UAnimInstance* Anim = Mesh != nullptr ? Mesh->GetAnimInstance() : nullptr;
	if (Anim == nullptr || !Anim->Montage_IsPlaying(PrimaryComboMontages[ComboStep]) ||
		!Mesh->DoesSocketExist(PrimaryTrailFirstSocket) ||
		!Mesh->DoesSocketExist(PrimaryTrailSecondSocket))
	{
		UE_LOG(LogTemp, Warning, TEXT("Knight trail skipped: montage or sockets unavailable. Actor[%s], Step[%d]"),
			*GetNameSafe(Character), ComboStep + 1);
		return;
	}

	if (PrimaryTrailComponent == nullptr)
	{
		PrimaryTrailComponent = NewObject<UParticleSystemComponent>(Mesh);
		PrimaryTrailComponent->bAutoActivate = false;
		PrimaryTrailComponent->bAutoDestroy = false;
		PrimaryTrailComponent->SetTemplate(PrimaryTrailParticle);
		PrimaryTrailComponent->SetupAttachment(Mesh);
		PrimaryTrailComponent->RegisterComponent();
	}

	PrimaryTrailComponent->BeginTrails(PrimaryTrailFirstSocket, PrimaryTrailSecondSocket,
		ETrailWidthMode_FromCentre, PrimaryTrailWidthScale);
	UE_LOG(LogTemp, Log,
		TEXT("Knight primary trail began. Actor[%s], Step[%d], Registered[%d], Active[%d], Emitters[%d], Parent[%s], Template[%s]"),
		*GetNameSafe(Character), ComboStep + 1, PrimaryTrailComponent->IsRegistered(),
		PrimaryTrailComponent->IsActive(), PrimaryTrailComponent->EmitterInstances.Num(),
		*GetNameSafe(PrimaryTrailComponent->GetAttachParent()), *GetNameSafe(PrimaryTrailComponent->Template));
}

void US1KnightCombatComponent::EndPrimaryTrail()
{
	if (PrimaryTrailComponent != nullptr && PrimaryTrailComponent->IsActive())
	{
		UE_LOG(LogTemp, Log,
			TEXT("Knight primary trail before end. Actor[%s], Step[%d], Particles[%d], RecentlyRendered[%d], BoundsOrigin[%s], BoundsExtent[%s]"),
			*GetNameSafe(GetOwner()), ComboStep + 1, PrimaryTrailComponent->GetNumActiveParticles(),
			PrimaryTrailComponent->WasRecentlyRendered(0.5f),
			*PrimaryTrailComponent->Bounds.Origin.ToString(),
			*PrimaryTrailComponent->Bounds.BoxExtent.ToString());
		PrimaryTrailComponent->EndTrails();
		UE_LOG(LogTemp, Log, TEXT("Knight primary trail ended. Actor[%s], Step[%d]"),
			*GetNameSafe(GetOwner()), ComboStep + 1);
	}
}

void US1KnightCombatComponent::StopPrimaryTrail()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PrimaryTrailStartTimer);
		World->GetTimerManager().ClearTimer(PrimaryTrailEndTimer);
	}
	EndPrimaryTrail();
}

void US1KnightCombatComponent::OnPrimaryMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// An older combo step can blend out after the next montage has already started.
	if (ActionState != ES1KnightActionState::PrimaryCombo ||
		!PrimaryComboMontages.IsValidIndex(ComboStep) || PrimaryComboMontages[ComboStep] != Montage)
	{
		return;
	}

	StopPrimaryTrail();
	if (bInterrupted)
	{
		FinishCurrentAction();
	}
}

void US1KnightCombatComponent::RejectPredictedPrimary()
{
	if (ActionState != ES1KnightActionState::PrimaryCombo)
		return;
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UAnimInstance* Anim = Character->GetMesh() != nullptr
			? Character->GetMesh()->GetAnimInstance() : nullptr)
		{
			if (PrimaryComboMontages.IsValidIndex(ComboStep))
				Anim->Montage_Stop(RejectBlendOutSeconds, PrimaryComboMontages[ComboStep]);
		}
	}
	FinishCurrentAction();
}

void US1KnightCombatComponent::InterruptByServer()
{
	if (ActionState == ES1KnightActionState::PrimaryCombo)
	{
		RejectPredictedPrimary();
		return;
	}
	if (ActionState == ES1KnightActionState::ChargePresentation)
	{
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		UAnimInstance* Anim = Character != nullptr && Character->GetMesh() != nullptr
			? Character->GetMesh()->GetAnimInstance() : nullptr;
		if (Anim != nullptr)
		{
			if (UAnimMontage* Montage = Cast<UAnimMontage>(ChargeAnimation))
			{
				Anim->Montage_Stop(0.10f, Montage);
			}
			else
			{
				Anim->StopSlotAnimation(0.10f, TEXT("UpperBody"));
			}
		}
	}
	FinishCurrentAction();
}

void US1KnightCombatComponent::LockMovementFacing()
{
	if (bMovementFacingLocked)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* Movement = Character != nullptr
		? Character->GetCharacterMovement()
		: nullptr;
	if (Movement == nullptr)
	{
		return;
	}

	bSavedOrientRotationToMovement = Movement->bOrientRotationToMovement;
	SavedMaxWalkSpeed = Movement->MaxWalkSpeed;
	Movement->bOrientRotationToMovement = false;
	bMovementFacingLocked = true;
}

void US1KnightCombatComponent::RestoreMovementFacing()
{
	if (!bMovementFacingLocked)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (UCharacterMovementComponent* Movement = Character != nullptr
		? Character->GetCharacterMovement()
		: nullptr)
	{
		Movement->bOrientRotationToMovement =
			bSavedOrientRotationToMovement;
		Movement->MaxWalkSpeed = SavedMaxWalkSpeed;
	}

	bMovementFacingLocked = false;
}

void US1KnightCombatComponent::ClearActionTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboBranchTimer);
		World->GetTimerManager().ClearTimer(ActionEndTimer);
	}
}

void US1KnightCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FinishCurrentAction();
	if (PrimaryTrailComponent != nullptr)
	{
		PrimaryTrailComponent->DestroyComponent();
		PrimaryTrailComponent = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}
