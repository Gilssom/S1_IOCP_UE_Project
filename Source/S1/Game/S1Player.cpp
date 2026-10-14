// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/S1Player.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "S1MyPlayer.h"
#include "S1CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Combat/S1KnightCombatComponent.h"
#include "UI/S1EntityHealthComponent.h"

// Sets default values
AS1Player::AS1Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<US1CharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(false);
	KnightCombatComponent = CreateDefaultSubobject<US1KnightCombatComponent>(TEXT("KnightCombatComponent"));
	HealthLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HealthLabel"));
	HealthLabel->SetupAttachment(GetRootComponent());
	HealthLabel->SetRelativeLocation(FVector(0.f, 0.f, 185.f));
	HealthLabel->SetHorizontalAlignment(EHTA_Center);
	HealthLabel->SetWorldSize(22.f);
	HealthLabel->SetText(FText::FromString(TEXT("Knight 100/100")));
	EntityHealth = CreateDefaultSubobject<US1EntityHealthComponent>(TEXT("EntityHealth"));
	EntityHealth->SetupAttachment(GetRootComponent());
	EntityHealth->SetRelativeLocation(FVector(0.f,0.f,185.f));

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	PlayerInfo = new Protocol::PosInfo();
	DestInfo = new Protocol::PosInfo();
}

AS1Player::~AS1Player()
{
	delete PlayerInfo;
	delete DestInfo;
	PlayerInfo = nullptr;
	DestInfo = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AS1Player::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	{
		FVector Location = GetActorLocation();
		DestInfo->set_x(Location.X);
		DestInfo->set_y(Location.Y);
		DestInfo->set_z(Location.Z);
		DestInfo->set_yaw(GetControlRotation().Yaw);

		SetMoveState(Protocol::MOVE_STATE_IDLE);
	}
}

void AS1Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	{
		FVector Location = GetActorLocation();
		PlayerInfo->set_x(Location.X);
		PlayerInfo->set_y(Location.Y);
		PlayerInfo->set_z(Location.Z);
		PlayerInfo->set_yaw(GetControlRotation().Yaw);
	}

	// 내 플레이어가 아닐 경우 (다른 플레이어일 경우)
	if (IsMyPlayer() == false)
	{
		const FVector CurrentLocation = GetActorLocation();
		const FVector TargetLocation(DestInfo->x(), DestInfo->y(), DestInfo->z());
		const float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);

		FVector NextLocation = CurrentLocation;
		if (DistanceToTarget >= RemoteSnapDistance)
		{
			// 큰 오차는 누적시키지 않고 서버가 전달한 좌표로 즉시 보정한다.
			NextLocation = TargetLocation;
		}
		else if (DistanceToTarget > RemoteArrivalTolerance)
		{
			// IDLE 상태여도 마지막 목적지에 도달할 때까지 계속 보정한다.
			NextLocation = FMath::VInterpTo(
				CurrentLocation,
				TargetLocation,
				DeltaTime,
				RemoteLocationInterpSpeed);
		}
		else
		{
			// VInterpTo의 점근 오차가 남지 않도록 목적지에 고정한다.
			NextLocation = TargetLocation;
		}

		if (DeltaTime > UE_SMALL_NUMBER)
		{
			RemoteVisualVelocity = (NextLocation - CurrentLocation) / DeltaTime;
			RemoteVisualVelocity = RemoteVisualVelocity.GetClampedToMaxSize(
				GetCharacterMovement()->GetMaxSpeed());
		}
		else
		{
			RemoteVisualVelocity = FVector::ZeroVector;
		}

		SetActorLocation(NextLocation, false);

		const bool bCombatFacingLocked =
			KnightCombatComponent != nullptr &&
			KnightCombatComponent->GetActionState() !=
				ES1KnightActionState::Ready;
		if (!bCombatFacingLocked)
		{
			const FRotator TargetRotation(0.0f, DestInfo->yaw(), 0.0f);
			const FRotator NextRotation = FMath::RInterpTo(
				GetActorRotation(),
				TargetRotation,
				DeltaTime,
				RemoteRotationInterpSpeed);
			SetActorRotation(NextRotation);
		}

		// ABP_Manny는 CharacterMovement의 Velocity와 Acceleration을 직접 읽는다.
		// 물리 이동 Tick은 비활성화하되 애니메이션용 이동 데이터는 제공한다.
		if (US1CharacterMovementComponent* MovementComponent =
			Cast<US1CharacterMovementComponent>(GetCharacterMovement()))
		{
			MovementComponent->SetNetworkAnimationData(RemoteVisualVelocity);
		}
	}
}

FVector AS1Player::GetVelocity() const
{
	if (IsMyPlayer() == false)
		return RemoteVisualVelocity;

	return Super::GetVelocity();
}

bool AS1Player::IsMyPlayer() const
{
	return Cast<const AS1MyPlayer>(this) != nullptr;
}

void AS1Player::SetMoveState(Protocol::MoveState State)
{
	if (PlayerInfo->state() == State)
	{
		ApplyRemoteAnimationMoveState(State);
		return;
	}

	const Protocol::MoveState PreviousState = PlayerInfo->state();
	PlayerInfo->set_state(State);
	ApplyRemoteAnimationMoveState(State);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Player movement state changed. ObjectId[%llu], Local[%s], State[%d -> %d]"),
		PlayerInfo->object_id(),
		IsMyPlayer() ? TEXT("true") : TEXT("false"),
		static_cast<int32>(PreviousState),
		static_cast<int32>(State));
}

void AS1Player::BeginPlay()
{
	Super::BeginPlay();
	HealthLabel->SetVisibility(bShowHealthDebugText);
	EntityHealth->SetPresentationEnabled(IsMyPlayer());
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* AS1Player::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AS1Player::ApplyRemoteAnimationMoveState(Protocol::MoveState State)
{
	if (IsMyPlayer())
		return;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent == nullptr)
		return;

	// Remote actors are moved directly from server snapshots while their
	// CharacterMovement tick is disabled. Keep only MovementMode in sync so
	// AnimBP queries such as IsFalling() still reconstruct the correct pose.
	const EMovementMode AnimationMovementMode =
		State == Protocol::MOVE_STATE_JUMP
		? MOVE_Falling
		: MOVE_Walking;

	if (MovementComponent->MovementMode != AnimationMovementMode)
	{
		MovementComponent->SetMovementMode(AnimationMovementMode);
	}
}

void AS1Player::SetPlayerInfo(const Protocol::PosInfo& Info)
{
	if (PlayerInfo->object_id() != 0)
	{
		assert(PlayerInfo->object_id() == Info.object_id());
	}

	// TODO
	PlayerInfo->CopyFrom(Info);
	DestInfo->CopyFrom(Info);
	RemoteVisualVelocity = FVector::ZeroVector;

	if (IsMyPlayer() == false)
	{
		UCharacterMovementComponent* MovementComponent =
			GetCharacterMovement();

		MovementComponent->StopMovementImmediately();

		// 원격 캐릭터는 네트워크 좌표로 직접 이동하므로
		// 로컬 물리 이동을 실행하지 않는다.
		MovementComponent->SetMovementMode(MOVE_Walking);
		MovementComponent->SetComponentTickEnabled(false);

		// 로컬/원격 플레이어 캡슐이 서로 밀어내지 않게 한다.
		GetCapsuleComponent()->SetCollisionResponseToChannel(
			ECC_Pawn,
			ECR_Ignore);

		if (US1CharacterMovementComponent* NetworkMovementComponent =
			Cast<US1CharacterMovementComponent>(
				MovementComponent))
		{
			NetworkMovementComponent->SetNetworkAnimationData(
				FVector::ZeroVector);
		}

		ApplyRemoteAnimationMoveState(Info.state());
	}

	FVector Location(Info.x(), Info.y(), Info.z());
	SetActorLocation(Location);
	SetActorRotation(FRotator(0.0f, Info.yaw(), 0.0f));
}

void AS1Player::ApplyServerHealth(uint32 CurrentHp, uint32 MaxHp, bool bDead)
{
	if(EntityHealth)EntityHealth->SetServerView(CurrentHp,MaxHp,bDead,FText::FromString(TEXT("Knight")));
	if (HealthLabel != nullptr)
	{
		HealthLabel->SetText(FText::FromString(FString::Printf(
			TEXT("Knight %u/%u%s"), CurrentHp, MaxHp,
			bDead ? TEXT(" [DEAD]") : TEXT(""))));
	}
}

void AS1Player::SetDestInfo(const Protocol::PosInfo& Info)
{
	if (PlayerInfo->object_id() != 0)
	{
		assert(PlayerInfo->object_id() == Info.object_id());
	}

	// TODO
	DestInfo->CopyFrom(Info);

	// 상태 적용
	SetMoveState(Info.state());
}
