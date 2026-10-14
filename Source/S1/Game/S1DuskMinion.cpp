#include "Game/S1DuskMinion.h"
#include "Game/S1DuskAnimInstance.h"
#include "Game/S1DuskDefinition.h"

#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UI/S1EntityHealthComponent.h"

AS1DuskMinion::AS1DuskMinion()
{
	PrimaryActorTick.bCanEverTick = true;
	CollisionRadius = S1DuskDefinition::CapsuleRadiusCm;
	CollisionHalfHeight = S1DuskDefinition::CapsuleHalfHeightCm;
	GetCapsuleComponent()->InitCapsuleSize(
		S1DuskDefinition::CapsuleRadiusCm,
		S1DuskDefinition::CapsuleHalfHeightCm);
	GetCharacterMovement()->DisableMovement();
	GetMesh()->SetRelativeLocation(FVector(
		0.f, 0.f, -S1DuskDefinition::CapsuleHalfHeightCm));
	GetMesh()->SetRelativeRotation(FRotator(0.f, MeshFacingYawOffset, 0.f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> DuskMesh(
		TEXT("/Game/ParagonMinions/Characters/Minions/Dusk_Minions/Meshes/Minion_Lane_Melee_Dusk.Minion_Lane_Melee_Dusk"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle(
		TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/NonCombat_Idle.NonCombat_Idle"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Jog(
		TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Combat_JogFwd.Combat_JogFwd"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Attack(
		TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Attack_A.Attack_A"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Death(
		TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/Death_A.Death_A"));
	static ConstructorHelpers::FClassFinder<US1DuskAnimInstance> DuskAnimBP(
		TEXT("/Game/S1/Monsters/ABP_S1Dusk"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DeathMaterial(
		TEXT("/Game/S1/Monsters/Materials/MI_S1Dusk_DeathFade.MI_S1Dusk_DeathFade"));
	DeathFadeMaterial = DeathMaterial.Object;

	if (DuskMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(DuskMesh.Object);
	}
	IdleSequence = Idle.Object;
	JogSequence = Jog.Object;
	AttackSequence = Attack.Object;
	DeathSequence = Death.Object;
	DuskAnimationClass = US1DuskAnimInstance::StaticClass();
	if (DuskAnimBP.Succeeded())
	{
		DuskAnimationClass = DuskAnimBP.Class;
	}
	GetMesh()->SetAnimInstanceClass(DuskAnimationClass);

	StatusLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("StatusLabel"));
	StatusLabel->SetupAttachment(GetRootComponent());
	StatusLabel->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	StatusLabel->SetHorizontalAlignment(EHTA_Center);
	StatusLabel->SetWorldSize(24.f);
	StatusLabel->SetText(FText::FromString(TEXT("Dusk")));
	EntityHealth=CreateDefaultSubobject<US1EntityHealthComponent>(TEXT("EntityHealth"));
	EntityHealth->SetupAttachment(GetRootComponent());
	EntityHealth->SetRelativeLocation(FVector(0.f,0.f,150.f));
	EntityHealth->bHideOnDeath=true;
	EntityHealth->SetPresentationEnabled(false);
}

void AS1DuskMinion::BeginPlay()
{
	Super::BeginPlay();
	StatusLabel->SetVisibility(bShowHealthDebugText);
	// Restore the blended class only if an older Blueprint serialized SingleNode mode.
	if (GetMesh()->GetAnimClass() != DuskAnimationClass)
	{
		GetMesh()->SetAnimInstanceClass(DuskAnimationClass);
	}
	if (US1DuskAnimInstance* Anim = GetDuskAnimInstance())
	{
		Anim->Configure(IdleSequence, JogSequence, AttackSequence, DeathSequence);
		Anim->SetBlendTimes(AttackEnterBlendSeconds, AttackExitBlendSeconds, LocomotionBlendSeconds);
		Anim->SetLocomotion(false);
	}
}

void AS1DuskMinion::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyBlueprintTuning();
}

void AS1DuskMinion::ApplyBlueprintTuning()
{
	// Collision is a mirror of the external server contract, not an artistic
	// Blueprint override. Scale and facing remain presentation tunables.
	CollisionRadius = S1DuskDefinition::CapsuleRadiusCm;
	CollisionHalfHeight = S1DuskDefinition::CapsuleHalfHeightCm;
	GetCapsuleComponent()->SetCapsuleSize(
		S1DuskDefinition::CapsuleRadiusCm,
		S1DuskDefinition::CapsuleHalfHeightCm);
	GetMesh()->SetRelativeLocation(FVector(
		0.f, 0.f, -S1DuskDefinition::CapsuleHalfHeightCm));
	GetMesh()->SetRelativeRotation(FRotator(0.f, MeshFacingYawOffset, 0.f));
	GetMesh()->SetRelativeScale3D(MeshVisualScale);
}

void AS1DuskMinion::Tick(float DeltaSeconds)
{
	RefreshHealthVisibility();
	Super::Tick(DeltaSeconds);
	UpdateDeathFade(DeltaSeconds);
	const bool bAttackPresentationActive = GetWorld() != nullptr &&
		GetWorld()->GetTimeSeconds() < AttackPresentationEndTime;
	if (GetWorld() != nullptr && AttackPresentationEndTime > 0.f &&
		GetWorld()->GetTimeSeconds() >= AttackPresentationEndTime)
	{
		AttackPresentationEndTime = 0.f;
		const int32 NextState = bHasPendingPresentationState ? PendingPresentationState : VisualState;
		bHasPendingPresentationState = false;
		ApplyPresentationState(NextState == 3 ? 0 : NextState);
	}
	if (VisualState != 5)
	{
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaSeconds, 12.f));
		if (!bAttackPresentationActive)
		{
			SetActorRotation(FMath::RInterpTo(GetActorRotation(),
				FRotator(0.f, TargetYaw, 0.f), DeltaSeconds, 12.f));
		}
	}
}

void AS1DuskMinion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// No timers, detached corpse, or global material state outlive this actor.
	bDeathPresentationComplete = true;
	DeathFadeMID = nullptr;
	AttackPresentationEndTime = 0.f;
	bHasPendingPresentationState = false;
	Super::EndPlay(EndPlayReason);
}

void AS1DuskMinion::ApplyInitialServerState(int32 State, uint32 CurrentHp, uint32 MaxHp)
{
	if (State != 5)
	{
		ApplyServerState(State, CurrentHp, MaxHp);
		return;
	}
	// DEAD snapshots have no death age. Never replay a fresh 3s death on reconnect.
	VisualState = State;
	Hp = CurrentHp;
	HpMax = MaxHp;
	bDeathPresentationStarted = true;
	bDeathPresentationComplete = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);
	RefreshLabel();
}

void AS1DuskMinion::UpdateDeathFade(float DeltaSeconds)
{
	if (!bDeathPresentationStarted || bDeathPresentationComplete) return;
	DeathPresentationElapsed += FMath::Max(0.f, DeltaSeconds);
	// Keep cosmetic completion inside the existing authoritative 3s lifetime,
	// even if Blueprint tunables are accidentally set beyond that budget.
	const float Delay = FMath::Clamp(DeathFadeDelaySeconds, 0.f, 2.89f);
	const float Duration = FMath::Clamp(DeathFadeDurationSeconds, 0.01f, 2.9f - Delay);
	if (DeathPresentationElapsed < Delay) return;
	if (DeathFadeMID == nullptr && DeathFadeMaterial != nullptr && GetMesh()->GetNumMaterials() == 1)
	{
		// The inspected Dusk has one slot (body, armour and weapon together).
		// Do not replace live material or mutate the shared MIC/parameter collection.
		DeathFadeMID = UMaterialInstanceDynamic::Create(DeathFadeMaterial, this);
		GetMesh()->SetMaterial(0, DeathFadeMID);
	}
	const float Progress = FMath::Clamp((DeathPresentationElapsed - Delay) / Duration, 0.f, 1.f);
	if (DeathFadeMID != nullptr)
	{
		// Existing MF_DeathFade opacity = 1 - saturate(2 * FadeOut).
		DeathFadeMID->SetScalarParameterValue(TEXT("FadeOut"), Progress * 0.5f);
	}
	if (Progress >= 1.f)
	{
		bDeathPresentationComplete = true;
		SetActorHiddenInGame(true); // Includes status label; removal still waits for S_DESPAWN.
	}
}

void AS1DuskMinion::ApplyServerTransform(const FVector& Location, float Yaw)
{
	TargetLocation = Location;
	TargetYaw = Yaw;
	if (FVector::DistSquared(GetActorLocation(), TargetLocation) > FMath::Square(300.f))
	{
		SetActorLocation(TargetLocation);
		SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
	}
}

US1DuskAnimInstance* AS1DuskMinion::GetDuskAnimInstance() const
{
	return GetMesh() != nullptr ? Cast<US1DuskAnimInstance>(GetMesh()->GetAnimInstance()) : nullptr;
}

void AS1DuskMinion::ApplyServerState(int32 State, uint32 CurrentHp, uint32 MaxHp)
{
	// A dead object ID cannot become alive again; respawn creates a fresh actor.
	if (bDeathPresentationStarted && State != 5) return;
	const bool bStateChanged = VisualState != State;
	VisualState = State;
	Hp = CurrentHp;
	HpMax = MaxHp;
	RefreshLabel();

	if (State == 5)
	{
		AttackPresentationEndTime = 0.f;
		bHasPendingPresentationState = false;
		if (!bDeathPresentationStarted)
		{
			bDeathPresentationStarted = true;
			DeathPresentationElapsed = 0.f;
			ApplyPresentationState(State);
		}
		return;
	}
	if (GetWorld() != nullptr &&
		GetWorld()->GetTimeSeconds() < AttackPresentationEndTime)
	{
		PendingPresentationState = State;
		bHasPendingPresentationState = true;
		return;
	}
	if (bStateChanged)
	{
		ApplyPresentationState(State);
	}
}

void AS1DuskMinion::ApplyPresentationState(int32 State)
{
	switch (State)
	{
	case 2: // MONSTER_STATE_CHASE
	case 4: // MONSTER_STATE_RETURN
		if (US1DuskAnimInstance* Anim = GetDuskAnimInstance()) Anim->SetLocomotion(true);
		break;
	case 5: // MONSTER_STATE_DEAD
		bHasPendingPresentationState = false;
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (US1DuskAnimInstance* Anim = GetDuskAnimInstance()) Anim->PlayDeath();
		break;
	case 3: // MONSTER_STATE_ATTACK: wait for S_MONSTER_ATTACK
		break;
	default:
		if (US1DuskAnimInstance* Anim = GetDuskAnimInstance()) Anim->SetLocomotion(false);
		break;
	}
}

void AS1DuskMinion::ApplyServerHealth(uint32 CurrentHp, uint32 MaxHp)
{
	Hp = CurrentHp;
	HpMax = MaxHp;
	RefreshLabel();
}

void AS1DuskMinion::PlayServerAttack(uint64 ActionInstanceId, float FacingYaw)
{
	if (ActionInstanceId == 0 || ActionInstanceId == LastAttackInstanceId ||
		VisualState == 5)
	{
		return;
	}

	LastAttackInstanceId = ActionInstanceId;
	bHasPendingPresentationState = false;
	SetActorRotation(FRotator(0.f, FacingYaw, 0.f));
	if (US1DuskAnimInstance* Anim = GetDuskAnimInstance()) Anim->PlayAttack();
	if (GetWorld() != nullptr && AttackSequence != nullptr)
	{
		AttackPresentationEndTime = GetWorld()->GetTimeSeconds() +
			AttackSequence->GetPlayLength() + AttackPresentationGraceSeconds;
	}
}

void AS1DuskMinion::RefreshLabel()
{
	RefreshHealthVisibility();
	if(EntityHealth)EntityHealth->SetServerView(Hp,HpMax,VisualState==5||Hp==0,FText::FromString(TEXT("Dusk")));
	if (StatusLabel == nullptr)
	{
		return;
	}

	const TCHAR* StateText = VisualState == 4 ? TEXT(" [RETURN]") :
		VisualState == 5 ? TEXT(" [DEAD]") : TEXT("");
	StatusLabel->SetText(FText::FromString(FString::Printf(
		TEXT("Dusk %u/%u%s"), Hp, HpMax, StateText)));
}

void AS1DuskMinion::SetLocalCombatTarget(bool bTargetsLocalPlayer)
{
	if(bInLocalCombat && !bTargetsLocalPlayer && GetWorld())
		HealthBarVisibleUntil=GetWorld()->GetTimeSeconds()+HealthBarCombatExitDelay;
	bInLocalCombat=bTargetsLocalPlayer;
	RefreshHealthVisibility();
}
void AS1DuskMinion::NotifyLocalCombatHit()
{
	if(GetWorld())HealthBarVisibleUntil=GetWorld()->GetTimeSeconds()+HealthBarCombatExitDelay;
	RefreshHealthVisibility();
}
void AS1DuskMinion::RefreshHealthVisibility()
{
	const bool bRecentHit=GetWorld() && GetWorld()->GetTimeSeconds()<HealthBarVisibleUntil;
	if(EntityHealth)EntityHealth->SetPresentationEnabled(bInLocalCombat||bRecentHit);
}
