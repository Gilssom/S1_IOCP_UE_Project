#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "S1KnightCombatComponent.generated.h"

class UAnimMontage;
class UAnimSequenceBase;
class UParticleSystem;
class UParticleSystemComponent;

UENUM(BlueprintType)
enum class ES1KnightActionState : uint8
{
	Ready,
	PrimaryCombo,
	ChargePresentation
};

UENUM()
enum class ES1KnightCombatAction : uint8
{
	PrimaryCombo,
	Charge
};

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FOnS1KnightActionStarted,
	ES1KnightCombatAction,
	int32,
	float);

/** Client presentation only. The external IOCP server remains combat-authoritative. */
UCLASS(ClassGroup=(S1), meta=(BlueprintSpawnableComponent))
class S1_API US1KnightCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	US1KnightCombatComponent();

	bool RequestPrimaryAttack(float FacingYaw);
	bool RequestChargePresentation(float FacingYaw);
	bool PlayReplicatedAction(
		ES1KnightCombatAction Action,
		int32 ReplicatedComboStep,
		uint32 Sequence,
		float FacingYaw);
	void InterruptByServer();
	void RejectPredictedPrimary();
	uint32 GetCurrentPredictionSequence() const { return CurrentPredictionSequence; }
	void SetCurrentPredictionSequence(uint32 Sequence) { CurrentPredictionSequence = Sequence; }

	FOnS1KnightActionStarted OnActionStarted;

	ES1KnightActionState GetActionState() const { return ActionState; }
	int32 GetComboStep() const { return ComboStep; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StartComboStep(int32 NewComboStep, float FacingYaw, bool bNotifyServer);
	bool StartChargePresentation(float FacingYaw, bool bNotifyServer);
	void ResolveComboBranch();
	void ResolveComboEnd();
	void StartBufferedNextStep();
	void FinishCurrentAction();
	void ClearActionTimers();
	void LockMovementFacing();
	void RestoreMovementFacing();
	bool CanStartGroundAction() const;
	void SchedulePrimaryTrail();
	void BeginPrimaryTrail();
	void EndPrimaryTrail();
	void StopPrimaryTrail();
	void OnPrimaryMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Combo")
	TArray<TObjectPtr<UAnimMontage>> PrimaryComboMontages;

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Combo", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ComboBranchRatio = 0.80f;

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Combo", meta=(ClampMin="0.0", ClampMax="1.0"))
	float AttackMovementSpeedMultiplier = 0.65f;
	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Combo", meta=(ClampMin="0.0", ClampMax="0.25"))
	float RejectBlendOutSeconds = 0.10f;

	/** Seconds on each A-D montage, measured from the beginning of that step. Client presentation only. */
	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Trail")
	TArray<FVector2D> PrimaryTrailWindows;

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Trail")
	TObjectPtr<UParticleSystem> PrimaryTrailParticle;

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Trail")
	FName PrimaryTrailFirstSocket = TEXT("FX_Trail_R_01");

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Trail")
	FName PrimaryTrailSecondSocket = TEXT("FX_Trail_R_02");

	UPROPERTY(EditDefaultsOnly, Category="Knight|Primary Trail", meta=(ClampMin="0.1", ClampMax="3.0"))
	float PrimaryTrailWidthScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category="Knight|Q")
	TObjectPtr<UAnimSequenceBase> ChargeAnimation;

	ES1KnightActionState ActionState = ES1KnightActionState::Ready;
	int32 ComboStep = INDEX_NONE;
	bool bNextComboBuffered = false;
	bool bComboBranchResolved = false;
	float BufferedFacingYaw = 0.0f;
	float CurrentActionDuration = 0.0f;
	float SavedMaxWalkSpeed = 0.0f;
	uint32 LastReplicatedActionSequence = 0;
	uint32 CurrentPredictionSequence = 0;
	bool bMovementFacingLocked = false;
	bool bSavedOrientRotationToMovement = true;
	UPROPERTY(Transient)
	TObjectPtr<UParticleSystemComponent> PrimaryTrailComponent;

	FTimerHandle ComboBranchTimer;
	FTimerHandle ActionEndTimer;
	FTimerHandle PrimaryTrailStartTimer;
	FTimerHandle PrimaryTrailEndTimer;
};
