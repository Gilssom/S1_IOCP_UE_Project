// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/S1Player.h"
#include "S1MyPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;
struct FInputActionValue;
enum class ES1KnightCombatAction : uint8;
namespace Protocol { class S_COMBAT_ACTION_RESULT; }

/**
 * 
 */
UCLASS()
class S1_API AS1MyPlayer : public AS1Player
{
	GENERATED_BODY()
	friend class FS1KnightQApprovalTest;
	friend class FS1QuickSlotInputTest;
	
public:
	AS1MyPlayer();
	void ApplyCombatActionResult(const Protocol::S_COMBAT_ACTION_RESULT& Packet);
	void ResetPredictedCombat(bool bResetSequence = false);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;
	virtual bool CanJumpInternal_Implementation() const override;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	void PrimaryAttack(const FInputActionValue& Value);
	void KnightSkillQ(const FInputActionValue& Value);
	void JumpWithCombatRules();
	void ZoomIn();
	void ZoomOut();
	void CycleTarget();
	void ClearTarget();
	void PickupPotion();
	void TogglePotionInventory();
	void EnsureQuickSlotInput();
	void UseQuickSlot(int32 Slot);
	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> QuickSlotContext;
	UPROPERTY(Transient) TArray<TObjectPtr<UInputAction>> QuickSlotActions;
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> QuickSlotInputOwner;
	void UpdateTargets(float DeltaTime);
	bool IsValidCombatTarget(const AActor* Candidate, bool bAllowOccluded) const;
	float ScoreTarget(const AActor* Candidate) const;
	bool CanUseGameplayInput() const;
	float PrepareCombatFacing();
	void HandleKnightActionStarted(
		ES1KnightCombatAction Action,
		int32 ComboStep,
		float FacingYaw);

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* KnightSkillQAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "250.0", ClampMax = "700.0"))
	float CameraDistance = 450.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "100.0"))
	float CameraMinDistance = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "250.0"))
	float CameraMaxDistance = 700.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "10.0"))
	float ZoomStep = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "1.0"))
	float CameraReturnSpeed = 8.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "0.1"))
	float MouseYawSensitivity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "0.1"))
	float MousePitchSensitivity = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat")
	bool bInvertMouseY = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "-89.0", ClampMax = "0.0"))
	float MinCameraPitch = -65.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float MaxCameraPitch = 35.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float CameraFOV = 80.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Combat", meta = (ClampMin = "0.0"))
	float CameraHeightAboveFeet = 130.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (ClampMin = "100.0"))
	float TargetSearchRange = 1800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (ClampMin = "100.0"))
	float TargetLockBreakRange = 2200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoftTargetMinimumDot = 0.65f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting", meta = (ClampMin = "0.0"))
	float TargetOcclusionGraceSeconds = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Targeting")
	bool bDrawTargetDebug = false;

protected:
	// 20Hz 전송으로 빠른 방향 전환 시 상대 클라이언트의 목표 좌표 지연을 줄인다.
	const float MOVE_PACKET_SEND_DELAY = 0.05f;
	float MovePacketSendTimer = MOVE_PACKET_SEND_DELAY;

	// Cache
	FVector2D DesiredInput;
	FVector DesiredMoveDirection;
	float DesiredYaw;

	// Dirty Flag Test
	FVector2D LastDesiredInput;
	uint32 CombatActionSequence = 0;
	struct FPredictedPrimary
	{
		uint32 Sequence = 0;
		uint32 ParentSequence = 0;
		uint32 ComboStep = 0;
		uint64 ActionInstanceId = 0;
		float StartedAt = 0.f;
		bool bAccepted = false;
		bool bTimedOut = false;
		bool bDiscarded = false;
	};
	TArray<FPredictedPrimary> PredictedPrimaryActions;
	struct FPredictedQ
	{
		uint32 Sequence = 0;
		float StartedAt = 0.f;
		bool bResolved = false;
		bool bTimedOut = false;
	};
	TArray<FPredictedQ> PredictedQActions;
	bool bInitialCameraPitchApplied = false;
	TWeakObjectPtr<AActor> SoftTarget;
	TWeakObjectPtr<AActor> LockedTarget;
	float TargetScanAccumulator = 0.0f;
	float LockedTargetOccludedSince = -1.0f;
};
