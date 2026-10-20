#pragma once

#include "CoreMinimal.h"
#include "Game/S1DuskDefinition.h"
#include "GameFramework/Character.h"
#include "S1DuskMinion.generated.h"

class UAnimSequence;
class US1DuskAnimInstance;
class UTextRenderComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class US1EntityHealthComponent;

// Visual representation only. The IOCP Room owns movement, actions, and HP.
UCLASS()
class S1_API AS1DuskMinion : public ACharacter
{
	GENERATED_BODY()

public:
	AS1DuskMinion();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ApplyInitialServerState(int32 State, uint32 CurrentHp, uint32 MaxHp);
	void ApplyServerState(int32 State, uint32 CurrentHp, uint32 MaxHp);
	void ApplyServerHealth(uint32 CurrentHp, uint32 MaxHp);
	void ApplyServerTransform(const FVector& Location, float Yaw);
	void PlayServerAttack(uint64 ActionInstanceId, float FacingYaw);
	void SetLocalCombatTarget(bool bTargetsLocalPlayer);
	void NotifyLocalCombatHit();
	bool IsThreateningLocalPlayer() const { return bInLocalCombat && Hp>0; }

private:
	US1DuskAnimInstance* GetDuskAnimInstance() const;
	void ApplyPresentationState(int32 State);
	void ApplyBlueprintTuning();
	void RefreshLabel();
	void RefreshHealthVisibility();
	UPROPERTY(EditDefaultsOnly, Category="Dusk|HP",meta=(ClampMin="0.0"))
	float HealthBarCombatExitDelay=3.f;
	bool bInLocalCombat=false;
	double HealthBarVisibleUntil=-1.0;
	void UpdateDeathFade(float DeltaSeconds);
	friend class FS1DuskDeathPresentationTest;

	// Cosmetic timing only; the external server still destroys the entity at 3s.
	UPROPERTY(EditDefaultsOnly, Category = "Dusk|Presentation|Death", meta = (ClampMin = "0.0", ClampMax = "2.9"))
	float DeathFadeDelaySeconds = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Dusk|Presentation|Death", meta = (ClampMin = "0.01", ClampMax = "2.9"))
	float DeathFadeDurationSeconds = 1.2f;
	UPROPERTY(EditDefaultsOnly, Category = "Dusk|Presentation|Death")
	TObjectPtr<UMaterialInterface> DeathFadeMaterial;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DeathFadeMID;
	float DeathPresentationElapsed = 0.f;
	bool bDeathPresentationStarted = false;
	bool bDeathPresentationComplete = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Presentation", meta = (AllowPrivateAccess = "true"))
	float MeshFacingYawOffset = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Presentation", meta = (AllowPrivateAccess = "true"))
	FVector MeshVisualScale = FVector(0.7f);

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Collision", meta = (AllowPrivateAccess = "true"))
	float CollisionRadius = S1DuskDefinition::CapsuleRadiusCm;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Collision", meta = (AllowPrivateAccess = "true"))
	float CollisionHalfHeight = S1DuskDefinition::CapsuleHalfHeightCm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Presentation", meta = (ClampMin = "0.0", ClampMax = "0.5", AllowPrivateAccess = "true"))
	float AttackPresentationGraceSeconds = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Presentation|Blend", meta = (ClampMin = "0.0", ClampMax = "0.5", AllowPrivateAccess = "true"))
	float AttackEnterBlendSeconds = 0.08f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Presentation|Blend", meta = (ClampMin = "0.0", ClampMax = "0.5", AllowPrivateAccess = "true"))
	float AttackExitBlendSeconds = 0.15f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dusk|Presentation|Blend", meta = (ClampMin = "0.0", ClampMax = "0.5", AllowPrivateAccess = "true"))
	float LocomotionBlendSeconds = 0.15f;

	UPROPERTY(VisibleAnywhere, Category = "Dusk|Presentation")
	TObjectPtr<UTextRenderComponent> StatusLabel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Dusk|HP",meta=(AllowPrivateAccess="true"))
	TObjectPtr<US1EntityHealthComponent> EntityHealth;
	UPROPERTY(EditDefaultsOnly, Category="Dusk|HP")
	bool bShowHealthDebugText=false;

	UPROPERTY()
	TObjectPtr<UAnimSequence> IdleSequence;
	UPROPERTY()
	TObjectPtr<UAnimSequence> JogSequence;
	UPROPERTY()
	TObjectPtr<UAnimSequence> AttackSequence;
	UPROPERTY()
	TObjectPtr<UAnimSequence> DeathSequence;
	UPROPERTY()
	TSubclassOf<US1DuskAnimInstance> DuskAnimationClass;

	int32 VisualState = 0;
	uint32 Hp = 0;
	uint32 HpMax = 0;
	uint64 LastAttackInstanceId = 0;
	FVector TargetLocation = FVector::ZeroVector;
	float TargetYaw = 0.f;
	float AttackPresentationEndTime = 0.f;
	int32 PendingPresentationState = 0;
	bool bHasPendingPresentationState = false;
};
