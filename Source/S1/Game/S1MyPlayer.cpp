// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/S1MyPlayer.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "S1.h"
#include "ClientPacketHandler.h"
#include "S1GameInstance.h"
#include "Kismet\KismetMathLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/S1KnightGameplayAbilities.h"
#include "Combat/S1KnightCombatComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "InputCoreTypes.h"
#include "Loot/S1InventorySubsystem.h"
#include "Quest/S1DialogueSubsystem.h"
#include "Quest/S1QuestSubsystem.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Camera/PlayerCameraManager.h"

// Sets default values
AS1MyPlayer::AS1MyPlayer()
{
	static ConstructorHelpers::FObjectFinder<UInputAction> AttackActionAsset(
		TEXT("/Game/Blueprints/Input/Actions/IA_Attack.IA_Attack"));
	static ConstructorHelpers::FObjectFinder<UInputAction> KnightSkillQActionAsset(
		TEXT("/Game/Blueprints/Input/Actions/IA_KnightQ.IA_KnightQ"));
	AttackAction = AttackActionAsset.Object;
	KnightSkillQAction = KnightSkillQActionAsset.Object;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	// Keep the camera's world rotation independent of transient capsule turns
	// between buffered combo steps; the controller still drives the boom.
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bDoCollisionTest = true;
	CameraBoom->ProbeSize = 12.0f;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = CameraReturnSpeed;
	CameraBoom->bEnableCameraRotationLag = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetFieldOfView(CameraFOV);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AS1MyPlayer::SnapCameraAfterServerPlacement()
{
	if(!CameraBoom)return;
	CameraBoom->UpdateComponentToWorld();
	const FVector ArmOrigin=CameraBoom->GetComponentLocation()+CameraBoom->TargetOffset;
	CameraBoom->PreviousDesiredLoc=ArmOrigin;
	CameraBoom->PreviousArmOrigin=ArmOrigin;
	CameraBoom->PreviousDesiredRot=CameraBoom->GetTargetRotation();
	CameraBoom->TickComponent(0.f,LEVELTICK_All,&CameraBoom->PrimaryComponentTick);
	if(const auto* PC=Cast<APlayerController>(GetController());PC&&PC->PlayerCameraManager)
		PC->PlayerCameraManager->SetGameCameraCutThisFrame();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AS1MyPlayer::NotifyControllerChanged()
{
	if(auto* Previous=QuickSlotInputOwner.Get())Previous->RemoveMappingContext(QuickSlotContext);
	QuickSlotInputOwner.Reset();
	Super::NotifyControllerChanged();
	DesiredYaw = GetActorRotation().Yaw;

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (!bInitialCameraPitchApplied)
		{
			FRotator InitialRotation = PlayerController->GetControlRotation();
			InitialRotation.Pitch = -15.0f;
			PlayerController->SetControlRotation(InitialRotation);
			bInitialCameraPitchApplied = true;
		}
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			EnsureQuickSlotInput();
			Subsystem->AddMappingContext(QuickSlotContext, 0);
			QuickSlotInputOwner=Subsystem;
		}
	}
}

void AS1MyPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnsureQuickSlotInput();
		for(int32 Slot=0;Slot<QuickSlotActions.Num();++Slot)
			EnhancedInputComponent->BindAction(QuickSlotActions[Slot],ETriggerEvent::Started,this,&AS1MyPlayer::UseQuickSlot,Slot);

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AS1MyPlayer::JumpWithCombatRules);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AS1MyPlayer::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AS1MyPlayer::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AS1MyPlayer::Look);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AS1MyPlayer::PrimaryAttack);
		EnhancedInputComponent->BindAction(KnightSkillQAction, ETriggerEvent::Started, this, &AS1MyPlayer::KnightSkillQ);
		PlayerInputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AS1MyPlayer::ZoomIn);
		PlayerInputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AS1MyPlayer::ZoomOut);
		PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AS1MyPlayer::CycleTarget);
		PlayerInputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AS1MyPlayer::ClearTarget);
		PlayerInputComponent->BindKey(EKeys::G, IE_Pressed, this, &AS1MyPlayer::PickupPotion);
		PlayerInputComponent->BindKey(EKeys::I, IE_Pressed, this, &AS1MyPlayer::TogglePotionInventory);
		PlayerInputComponent->BindKey(EKeys::O, IE_Pressed, this, &AS1MyPlayer::ToggleQuestJournal);
		PlayerInputComponent->BindKey(EKeys::LeftAlt, IE_Pressed, this, &AS1MyPlayer::BeginHUDCursor);
		PlayerInputComponent->BindKey(EKeys::LeftAlt, IE_Released, this, &AS1MyPlayer::EndHUDCursor);
		PlayerInputComponent->BindKey(EKeys::RightAlt, IE_Pressed, this, &AS1MyPlayer::BeginHUDCursor);
		PlayerInputComponent->BindKey(EKeys::RightAlt, IE_Released, this, &AS1MyPlayer::EndHUDCursor);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AS1MyPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHUDCursor();
	UpdateTargets(DeltaTime);
	const float Now = static_cast<float>(GetWorld()->GetTimeSeconds());
	for (FPredictedPrimary& Prediction : PredictedPrimaryActions)
	{
		if (!Prediction.bAccepted && !Prediction.bDiscarded &&
			!Prediction.bTimedOut && Now - Prediction.StartedAt >= 2.0f)
		{
			Prediction.bTimedOut = true;
			UE_LOG(LogTemp, Warning,
				TEXT("Combat response delayed; outcome unknown. Sequence[%u], Step[%u]"),
				Prediction.Sequence, Prediction.ComboStep);
		}
	}

	// Send Check
	for (FPredictedQ& Prediction : PredictedQActions)
	{
		if (!Prediction.bResolved && !Prediction.bTimedOut && Now - Prediction.StartedAt >= 2.f)
		{
			Prediction.bTimedOut = true;
			UE_LOG(LogTemp, Warning, TEXT("Q response delayed; outcome unknown. Sequence[%u]"), Prediction.Sequence);
		}
	}

	// Send Check
	bool ForceSendPacket = false;

	if (LastDesiredInput != DesiredInput)
	{
		ForceSendPacket = true;
		LastDesiredInput = DesiredInput;
	}

	// Locomotion state is part of the movement packet. Resolve airborne state
	// before ground input so a running jump is not reported as RUN.
	Protocol::MoveState DesiredMoveState = Protocol::MOVE_STATE_IDLE;
	if (GetCharacterMovement()->IsFalling())
	{
		DesiredMoveState = Protocol::MOVE_STATE_JUMP;
	}
	else if (DesiredInput.IsNearlyZero() == false)
	{
		DesiredMoveState = Protocol::MOVE_STATE_RUN;
	}

	if (GetMoveState() != DesiredMoveState)
	{
		SetMoveState(DesiredMoveState);
		ForceSendPacket = true;
	}

	MovePacketSendTimer -= DeltaTime;

	if (MovePacketSendTimer <= 0 || ForceSendPacket)
	{
		MovePacketSendTimer = MOVE_PACKET_SEND_DELAY;

		Protocol::C_MOVE MovePkt;

		// 현재 위치 정보
		{
			// 내부적으로 PlayerInfo 를 설정
			Protocol::PosInfo* Info = MovePkt.mutable_info();
			Info->CopyFrom(*PlayerInfo);
			Info->set_yaw(DesiredYaw);
			Info->set_state(GetMoveState());
		}

		if (US1GameInstance* GameInstance =
			Cast<US1GameInstance>(
				GetGameInstance()))
		{
			SendBufferRef SendBuffer =
				ClientPacketHandler::MakeSendBuffer(
					MovePkt);
			GameInstance->SendPacket(SendBuffer);
		}
	}
}

void AS1MyPlayer::BeginPlay()
{
	Super::BeginPlay();
	// Existing Blueprint component overrides must not reintroduce parent yaw inheritance.
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f,
		CameraHeightAboveFeet - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	CameraBoom->TargetArmLength = FMath::Clamp(CameraDistance, CameraMinDistance, CameraMaxDistance);
	CameraBoom->CameraLagSpeed = CameraReturnSpeed;
	FollowCamera->SetFieldOfView(CameraFOV);

	if (US1KnightCombatComponent* Combat = GetKnightCombatComponent())
	{
		Combat->OnActionStarted.AddUObject(
			this,
			&AS1MyPlayer::HandleKnightActionStarted);
	}

	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent())
	{
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(US1KnightPrimaryComboAbility::StaticClass(), 1));
		AbilitySystem->GiveAbility(FGameplayAbilitySpec(US1KnightChargeAbility::StaticClass(), 1));
	}
}

void AS1MyPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>())Dialogue->AbortDialogue();
	if(auto* InputOwner=QuickSlotInputOwner.Get())InputOwner->RemoveMappingContext(QuickSlotContext);
	QuickSlotInputOwner.Reset();
	ResetPredictedCombat();
	SoftTarget.Reset();
	LockedTarget.Reset();
	if (US1KnightCombatComponent* Combat = GetKnightCombatComponent())
	{
		Combat->OnActionStarted.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AS1MyPlayer::EnsureQuickSlotInput()
{
	if(QuickSlotContext)return;
	QuickSlotContext=NewObject<UInputMappingContext>(this);
	const FKey Primary[4]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four};
	const FKey Numpad[4]={EKeys::NumPadOne,EKeys::NumPadTwo,EKeys::NumPadThree,EKeys::NumPadFour};
	for(int32 Slot=0;Slot<4;++Slot)
	{
		auto* Action=NewObject<UInputAction>(this);Action->ValueType=EInputActionValueType::Boolean;
		QuickSlotActions.Add(Action);
		for(const FKey& Key:{Primary[Slot],Numpad[Slot]})
		{
			const bool bConflict=DefaultMappingContext&&DefaultMappingContext->GetMappings().ContainsByPredicate(
				[Key](const FEnhancedActionKeyMapping& Mapping){return Mapping.Key==Key;});
			if(bConflict){UE_LOG(LogTemp,Warning,TEXT("Quick-slot key skipped: existing mapping uses %s"),*Key.ToString());continue;}
			QuickSlotContext->MapKey(Action,Key);
		}
	}
}
void AS1MyPlayer::UseQuickSlot(int32 Slot)
{
	if(!CanUseGameplayInput())return;
	if(auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>())Inventory->RequestQuickSlot(Slot);
}

void AS1MyPlayer::HandleKnightActionStarted(
	ES1KnightCombatAction Action,
	int32 ComboStep,
	float FacingYaw)
{
	DesiredYaw = FacingYaw;

	Protocol::CombatActionType ActionType = Protocol::COMBAT_ACTION_NONE;
	switch (Action)
	{
	case ES1KnightCombatAction::PrimaryCombo:
		ActionType = Protocol::COMBAT_ACTION_KNIGHT_PRIMARY;
		break;
	case ES1KnightCombatAction::Charge:
		ActionType = Protocol::COMBAT_ACTION_KNIGHT_Q;
		ComboStep = 0;
		break;
	}

	if (ActionType == Protocol::COMBAT_ACTION_NONE)
	{
		return;
	}

	++CombatActionSequence;
	if (CombatActionSequence == 0)
	{
		++CombatActionSequence;
	}

	Protocol::C_COMBAT_ACTION CombatPacket;
	CombatPacket.set_action_type(ActionType);
	CombatPacket.set_combo_step(static_cast<uint32>(ComboStep));
	CombatPacket.set_sequence(CombatActionSequence);
	CombatPacket.set_facing_yaw(FacingYaw);
	if (ActionType == Protocol::COMBAT_ACTION_KNIGHT_PRIMARY)
	{
		if (US1KnightCombatComponent* Combat = GetKnightCombatComponent())
		{
			const uint32 ParentSequence = Combat->GetCurrentPredictionSequence();
			PredictedPrimaryActions.Add({CombatActionSequence, ParentSequence,
				static_cast<uint32>(ComboStep), 0,
				static_cast<float>(GetWorld()->GetTimeSeconds()), false, false, false});
			if (PredictedPrimaryActions.Num() > 32)
				PredictedPrimaryActions.RemoveAt(0);
			Combat->SetCurrentPredictionSequence(CombatActionSequence);
		}
		CombatPacket.set_combat_contract_version(1);
	}
	else if (ActionType == Protocol::COMBAT_ACTION_KNIGHT_Q)
	{
		PredictedQActions.Add({CombatActionSequence, static_cast<float>(GetWorld()->GetTimeSeconds()), false, false});
		if (PredictedQActions.Num() > 32)
			PredictedQActions.RemoveAt(0);
		if (US1KnightCombatComponent* Combat = GetKnightCombatComponent())
			Combat->SetCurrentPredictionSequence(CombatActionSequence);
		CombatPacket.set_combat_contract_version(2);
	}

	if (US1GameInstance* GameInstance = Cast<US1GameInstance>(GetGameInstance()))
	{
		GameInstance->SendPacket(
			ClientPacketHandler::MakeSendBuffer(CombatPacket));
		UE_LOG(LogTemp, Log,
			TEXT("Combat action sent. Actor[%s], Type[%d], Step[%d], Sequence[%u], FacingYaw[%.2f]"),
			*GetNameSafe(this),
			static_cast<int32>(ActionType),
			ComboStep,
			CombatActionSequence,
			FacingYaw);
	}
}

float AS1MyPlayer::PrepareCombatFacing()
{
	const float CameraYaw = Controller != nullptr
		? Controller->GetControlRotation().Yaw
		: GetActorRotation().Yaw;

	DesiredYaw = CameraYaw;
	SetActorRotation(FRotator(0.0f, CameraYaw, 0.0f));
	return CameraYaw;
}

void AS1MyPlayer::PrimaryAttack(const FInputActionValue& Value)
{
	if (!CanUseGameplayInput()) return;
	const US1GameInstance* GameInstance = Cast<US1GameInstance>(GetGameInstance());
	if (GameInstance == nullptr || !GameInstance->SupportsCombatApproval())
		return;
	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent())
	{
		AbilitySystem->TryActivateAbilityByClass(US1KnightPrimaryComboAbility::StaticClass());
	}
}

void AS1MyPlayer::ApplyCombatActionResult(
	const Protocol::S_COMBAT_ACTION_RESULT& Packet)
{
	if (Packet.action_type() == Protocol::COMBAT_ACTION_KNIGHT_Q)
	{
		FPredictedQ* Prediction = PredictedQActions.FindByPredicate(
			[&](const FPredictedQ& Entry) { return Entry.Sequence == Packet.sequence(); });
		if (Prediction == nullptr || Prediction->bResolved || Packet.combo_step() != 0)
			return;
		if (Packet.decision() == Protocol::COMBAT_DECISION_ACCEPTED)
		{
			if (Packet.action_instance_id() == 0) return;
			Prediction->bResolved = true;
			UE_LOG(LogTemp, Log, TEXT("Q accepted. Sequence[%u], Action[%llu]"), Packet.sequence(), Packet.action_instance_id());
		}
		else if (Packet.decision() == Protocol::COMBAT_DECISION_REJECTED)
		{
			Prediction->bResolved = true;
			UE_LOG(LogTemp, Warning, TEXT("Q rejected. Sequence[%u], Reason[%d]"), Packet.sequence(), static_cast<int32>(Packet.reason()));
			if (US1KnightCombatComponent* Combat = GetKnightCombatComponent())
				if (Combat->GetActionState() == ES1KnightActionState::ChargePresentation &&
					Combat->GetCurrentPredictionSequence() == Packet.sequence())
					Combat->InterruptByServer();
		}
		return;
	}
	if (Packet.action_type() != Protocol::COMBAT_ACTION_KNIGHT_PRIMARY)
		return;
	FPredictedPrimary* Found = PredictedPrimaryActions.FindByPredicate(
		[&](const FPredictedPrimary& Entry) { return Entry.Sequence == Packet.sequence(); });
	if (Found == nullptr || Found->ComboStep != Packet.combo_step() ||
		Found->bAccepted || Found->bDiscarded)
		return;
	if (Packet.decision() == Protocol::COMBAT_DECISION_ACCEPTED)
	{
		if (Packet.action_instance_id() == 0)
			return;
		Found->bAccepted = true;
		Found->ActionInstanceId = Packet.action_instance_id();
		UE_LOG(LogTemp, Log, TEXT("Combat accepted. Sequence[%u], Action[%llu], Step[%u]"),
			Packet.sequence(), Packet.action_instance_id(), Packet.combo_step());
		return;
	}
	if (Packet.decision() != Protocol::COMBAT_DECISION_REJECTED)
		return;
	Found->bDiscarded = true;
	UE_LOG(LogTemp, Warning, TEXT("Combat rejected. Sequence[%u], Step[%u], Reason[%d]"),
		Packet.sequence(), Packet.combo_step(), static_cast<int32>(Packet.reason()));
	US1KnightCombatComponent* Combat = GetKnightCombatComponent();
	uint32 Current = Combat != nullptr ? Combat->GetCurrentPredictionSequence() : 0;
	bool bDependent = false;
	for (int32 Depth = 0; Current != 0 && Depth < 32; ++Depth)
	{
		FPredictedPrimary* Entry = PredictedPrimaryActions.FindByPredicate(
			[&](const FPredictedPrimary& Candidate) { return Candidate.Sequence == Current; });
		if (Entry == nullptr || Entry->bAccepted)
			break;
		if (Current == Packet.sequence())
		{
			bDependent = true;
			break;
		}
		Current = Entry->ParentSequence;
	}
	if (bDependent && Combat != nullptr)
	{
		Combat->RejectPredictedPrimary();
		for (FPredictedPrimary& Entry : PredictedPrimaryActions)
			if (!Entry.bAccepted && Entry.Sequence >= Packet.sequence())
				Entry.bDiscarded = true;
	}
}

void AS1MyPlayer::ResetPredictedCombat(bool bResetSequence)
{
	PredictedPrimaryActions.Reset();
	PredictedQActions.Reset();
	if (bResetSequence)
		CombatActionSequence = 0;
	if (US1KnightCombatComponent* Combat = GetKnightCombatComponent())
		Combat->InterruptByServer();
}

void AS1MyPlayer::KnightSkillQ(const FInputActionValue& Value)
{
	if (!CanUseGameplayInput()) return;
	const US1GameInstance* GameInstance = Cast<US1GameInstance>(GetGameInstance());
	if (GameInstance == nullptr || !GameInstance->SupportsKnightQApproval()) return;
	const US1KnightCombatComponent* Combat = GetKnightCombatComponent();
	if (Combat == nullptr || Combat->GetActionState() != ES1KnightActionState::Ready) return;
	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent())
	{
		PrepareCombatFacing();
		AbilitySystem->TryActivateAbilityByClass(US1KnightChargeAbility::StaticClass());
	}
}

void AS1MyPlayer::JumpWithCombatRules()
{
	if (!CanUseGameplayInput()) return;
	const US1KnightCombatComponent* Combat = GetKnightCombatComponent();
	if (Combat != nullptr && Combat->GetActionState() == ES1KnightActionState::ChargePresentation)
		return;
	Jump();
}

bool AS1MyPlayer::CanJumpInternal_Implementation() const
{
	const US1KnightCombatComponent* Combat = GetKnightCombatComponent();
	return (Combat == nullptr || Combat->GetActionState() != ES1KnightActionState::ChargePresentation)
		&& Super::CanJumpInternal_Implementation();
}

void AS1MyPlayer::Move(const FInputActionValue& Value)
{
	if (!CanUseGameplayInput()) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		// Cache
		{
			DesiredInput = MovementVector;

			// 입력 해제 시에는 마지막 유효 이동 방향을 유지한다.
			// ZeroVector로 LookAt을 계산하면 Yaw가 0으로 되돌아간다.
			if (MovementVector.IsNearlyZero() == false)
			{
				DesiredMoveDirection = FVector::ZeroVector;
				DesiredMoveDirection += ForwardDirection * MovementVector.Y;
				DesiredMoveDirection += RightDirection * MovementVector.X;
				DesiredMoveDirection.Normalize();

				const FVector Location = GetActorLocation();
				const FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(
					Location,
					Location + DesiredMoveDirection);
				DesiredYaw = Rotator.Yaw;
			}
		}
	}
}

void AS1MyPlayer::Look(const FInputActionValue& Value)
{
	if (!CanUseGameplayInput()) return;
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X * MouseYawSensitivity);
		const float PitchSign = bInvertMouseY ? -1.0f : 1.0f;
		const float CurrentPitch = FRotator::NormalizeAxis(Controller->GetControlRotation().Pitch);
		const float DesiredPitch = FMath::Clamp(
			CurrentPitch + LookAxisVector.Y * MousePitchSensitivity * PitchSign,
			MinCameraPitch, MaxCameraPitch);
		AddControllerPitchInput((DesiredPitch - CurrentPitch));
	}
}

bool AS1MyPlayer::CanUseGameplayInput() const
{
	if(const auto* Instance=GetGameInstance<US1GameInstance>();Instance&&Instance->IsPortalTransferActive())return false;
	if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>();Dialogue&&Dialogue->IsModal())return false;
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	return PlayerController != nullptr && !PlayerController->bShowMouseCursor &&
		!PlayerController->IsPaused();
}

void AS1MyPlayer::PickupPotion()
{
 if(CanUseGameplayInput())if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>())Dialogue->Interact();
}
void AS1MyPlayer::StopForDialogue()
{
 DesiredInput=FVector2D::ZeroVector;ConsumeMovementInputVector();StopJumping();
 GetCharacterMovement()->StopMovementImmediately();
}
void AS1MyPlayer::TogglePotionInventory()
{
 if(auto* Quests=GetGameInstance()->GetSubsystem<US1QuestSubsystem>();Quests&&Quests->IsJournalOpen())return;
 if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>();Dialogue&&Dialogue->IsModal())return;
 if(auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>())
 {
  DesiredInput=FVector2D::ZeroVector;
  ConsumeMovementInputVector();
  Inventory->ToggleInventory();
 }
}
void AS1MyPlayer::ToggleQuestJournal()
{
 if(auto* Quests=GetGameInstance()->GetSubsystem<US1QuestSubsystem>())Quests->ToggleJournal();
}
void AS1MyPlayer::BeginHUDCursor()
{
 if(!CanUseGameplayInput())return;
 bHUDCursorHeld=true;StopForDialogue();UpdateHUDCursor();
}
void AS1MyPlayer::EndHUDCursor()
{
 if(!bHUDCursorHeld)return;bHUDCursorHeld=false;
 auto* PC=Cast<APlayerController>(GetController());if(!PC||!GetGameInstance())return;
 const auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>();
 const auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>();
 const auto* Quests=GetGameInstance()->GetSubsystem<US1QuestSubsystem>();
 if((Dialogue&&Dialogue->IsModal())||(Inventory&&Inventory->IsInventoryOpen())||(Quests&&Quests->IsJournalOpen()))return;
 PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());
}
void AS1MyPlayer::UpdateHUDCursor()
{
 if(!bHUDCursorHeld)return;
 // UIOnly windows may consume key-up; Slate modifiers also release on focus loss.
 if(FSlateApplication::IsInitialized()&&!FSlateApplication::Get().GetModifierKeys().IsAltDown())
 {EndHUDCursor();return;}
 auto* PC=Cast<APlayerController>(GetController());if(!PC||!PC->IsLocalController()||!GetGameInstance())return;
 const auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>();
 const auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>();
 const auto* Quests=GetGameInstance()->GetSubsystem<US1QuestSubsystem>();
 if((Dialogue&&Dialogue->IsModal())||(Inventory&&Inventory->IsInventoryOpen())||(Quests&&Quests->IsJournalOpen()))return;
 if(!PC->bShowMouseCursor)
 {
  FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
  PC->SetInputMode(Mode);PC->bShowMouseCursor=true;
 }
}

void AS1MyPlayer::ZoomIn()
{
	if (CanUseGameplayInput())
	{
		CameraBoom->TargetArmLength = FMath::Clamp(
			CameraBoom->TargetArmLength - ZoomStep, CameraMinDistance, CameraMaxDistance);
	}
}

void AS1MyPlayer::ZoomOut()
{
	if (CanUseGameplayInput())
	{
		CameraBoom->TargetArmLength = FMath::Clamp(
			CameraBoom->TargetArmLength + ZoomStep, CameraMinDistance, CameraMaxDistance);
	}
}

bool AS1MyPlayer::IsValidCombatTarget(const AActor* Candidate, bool bAllowOccluded) const
{
	if (!IsValid(Candidate) || Candidate == this ||
		!Candidate->ActorHasTag(TEXT("S1.TargetableEnemy")) ||
		Candidate->ActorHasTag(TEXT("S1.Dead")) ||
		Candidate->ActorHasTag(TEXT("S1.Friendly")) ||
		FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation()) >
			FMath::Square(TargetLockBreakRange))
	{
		return false;
	}

	if (bAllowOccluded || GetWorld() == nullptr || FollowCamera == nullptr)
	{
		return true;
	}

	FHitResult Hit;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(S1TargetSight), false, this);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, FollowCamera->GetComponentLocation(), Candidate->GetActorLocation(),
		ECC_Visibility, Query);
	return !bHit || Hit.GetActor() == Candidate;
}

float AS1MyPlayer::ScoreTarget(const AActor* Candidate) const
{
	if (FollowCamera == nullptr) return -1.0f;
	const FVector ToTarget = (Candidate->GetActorLocation() -
		FollowCamera->GetComponentLocation()).GetSafeNormal();
	const float Dot = FVector::DotProduct(FollowCamera->GetForwardVector(), ToTarget);
	if (Dot < SoftTargetMinimumDot) return -1.0f;
	const float Distance = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
	return Dot * 2.0f - Distance / TargetSearchRange * 0.5f;
}

void AS1MyPlayer::UpdateTargets(float DeltaTime)
{
	if (!IsLocallyControlled() || GetWorld() == nullptr) return;
	TargetScanAccumulator += DeltaTime;
	if (TargetScanAccumulator < 0.10f) return;
	TargetScanAccumulator = 0.0f;

	if (AActor* Locked = LockedTarget.Get())
	{
		if (!IsValidCombatTarget(Locked, true))
		{
			ClearTarget();
		}
		else if (IsValidCombatTarget(Locked, false))
		{
			LockedTargetOccludedSince = -1.0f;
		}
		else
		{
			if (LockedTargetOccludedSince < 0.0f)
				LockedTargetOccludedSince = GetWorld()->GetTimeSeconds();
			else if (GetWorld()->GetTimeSeconds() - LockedTargetOccludedSince >
				TargetOcclusionGraceSeconds)
				ClearTarget();
		}
	}

	AActor* Best = nullptr;
	float BestScore = SoftTarget.IsValid() && IsValidCombatTarget(SoftTarget.Get(), false)
		? ScoreTarget(SoftTarget.Get()) + 0.15f : -1.0f;
	if (BestScore >= 0.0f) Best = SoftTarget.Get();
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Candidate = *It;
		if (!IsValidCombatTarget(Candidate, false)) continue;
		const float Distance = FVector::Dist(GetActorLocation(), Candidate->GetActorLocation());
		if (Distance > TargetSearchRange) continue;
		const float Score = ScoreTarget(Candidate);
		if (Score > BestScore) { Best = Candidate; BestScore = Score; }
	}
	SoftTarget = Best;

	if (bDrawTargetDebug)
	{
		AActor* DisplayTarget = LockedTarget.IsValid() ? LockedTarget.Get() : SoftTarget.Get();
		if (DisplayTarget != nullptr)
		{
			DrawDebugSphere(GetWorld(), DisplayTarget->GetActorLocation() + FVector(0, 0, 100),
				24.0f, 12, LockedTarget.IsValid() ? FColor::Yellow : FColor::Cyan,
				false, 0.12f);
		}
	}
}

void AS1MyPlayer::CycleTarget()
{
	if (!CanUseGameplayInput() || GetWorld() == nullptr) return;
	TArray<AActor*> Candidates;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Candidate = *It;
		if (IsValidCombatTarget(Candidate, false) &&
			FVector::Dist(GetActorLocation(), Candidate->GetActorLocation()) <= TargetSearchRange &&
			ScoreTarget(Candidate) >= 0.0f)
			Candidates.Add(Candidate);
	}
	Candidates.Sort([this](const AActor& A, const AActor& B) {
		return ScoreTarget(&A) > ScoreTarget(&B);
	});
	if (Candidates.IsEmpty()) { ClearTarget(); return; }
	const int32 CurrentIndex = Candidates.IndexOfByKey(LockedTarget.Get());
	LockedTarget = Candidates[(CurrentIndex + 1) % Candidates.Num()];
	LockedTargetOccludedSince = -1.0f;
	UE_LOG(LogTemp, Log, TEXT("Combat target locked: %s"), *GetNameSafe(LockedTarget.Get()));
}

void AS1MyPlayer::ClearTarget()
{
	if(auto* Inventory=GetGameInstance()->GetSubsystem<US1InventorySubsystem>();Inventory&&Inventory->IsInventoryOpen())
	{Inventory->CloseInventory();return;}
	LockedTarget.Reset();
	LockedTargetOccludedSince = -1.0f;
}

