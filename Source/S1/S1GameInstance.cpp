// Fill out your copyright notice in the Description page of Project Settings.


#include "S1GameInstance.h"
#include "Network/S1NetworkSubsystem.h"
#include "Protocol.pb.h"
#include "World/S1WorldEntitySubsystem.h"
#include "S1AuthWidget.h"
#include "S1CharacterSelectWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Loot/S1InventorySubsystem.h"
#include "Level/S1RegionContract.h"

void US1GameInstance::Init()
{
	Super::Init();

	if (US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		NetworkSubsystem->OnConnectionStateChanged.AddUObject(
			this, &US1GameInstance::HandleCombatConnectionState);
		NetworkSubsystem->OnEnterGamePacket.AddUObject(
			this,
			&US1GameInstance::HandleEnterGamePacket);
		NetworkSubsystem->OnLeaveGamePacket.AddUObject(
			this,
			&US1GameInstance::HandleLeaveGamePacket);
		NetworkSubsystem->OnSpawnPacket.AddUObject(
			this,
			&US1GameInstance::HandleSpawnPacket);
		NetworkSubsystem->OnDespawnPacket.AddUObject(
			this,
			&US1GameInstance::HandleDespawnPacket);
		NetworkSubsystem->OnMovePacket.AddUObject(
			this,
			&US1GameInstance::HandleMovePacket);
		NetworkSubsystem->OnChatPacket.AddUObject(
			this,
			&US1GameInstance::HandleChatPacket);
		NetworkSubsystem->OnCombatActionPacket.AddUObject(
			this,
			&US1GameInstance::HandleCombatActionPacket);
		NetworkSubsystem->OnCombatActionResultPacket.AddUObject(
			this, &US1GameInstance::HandleCombatActionResultPacket);
		NetworkSubsystem->OnMonsterUpdatePacket.AddUObject(this, &US1GameInstance::HandleMonsterUpdatePacket);
		NetworkSubsystem->OnMonsterAttackPacket.AddUObject(this, &US1GameInstance::HandleMonsterAttackPacket);
		NetworkSubsystem->OnDamageResultPacket.AddUObject(this, &US1GameInstance::HandleDamageResultPacket);
		NetworkSubsystem->OnPlayerStatusPacket.AddUObject(this, &US1GameInstance::HandlePlayerStatusPacket);
		NetworkSubsystem->OnPlayerHealedPacket.AddUObject(this,&US1GameInstance::HandlePlayerHealedPacket);
		NetworkSubsystem->OnLootSnapshotPacket.AddUObject(this,&US1GameInstance::HandleLootSnapshotPacket);
		NetworkSubsystem->OnLootCreatedPacket.AddUObject(this,&US1GameInstance::HandleLootCreatedPacket);
		NetworkSubsystem->OnLootRemovedPacket.AddUObject(this,&US1GameInstance::HandleLootRemovedPacket);
	}

	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->OnLoginCompleted.AddDynamic(
			this,
			&US1GameInstance::HandleAccountLoginCompleted);
		AccountSubsystem->OnSignUpCompleted.AddDynamic(
			this,
			&US1GameInstance::HandleAccountSignUpCompleted);
		AccountSubsystem->OnCharacterCreated.AddDynamic(
			this,
			&US1GameInstance::HandleAccountCharacterCreated);
		AccountSubsystem->OnEnterGameCompleted.AddDynamic(
			this,
			&US1GameInstance::HandleAccountEnterGameCompleted);
	}
}

void US1GameInstance::Shutdown()
{
	if(auto* Inventory=GetSubsystem<US1InventorySubsystem>())Inventory->ResetSession();
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		NetworkSubsystem->OnEnterGamePacket.RemoveAll(this);
		NetworkSubsystem->OnLeaveGamePacket.RemoveAll(this);
		NetworkSubsystem->OnSpawnPacket.RemoveAll(this);
		NetworkSubsystem->OnDespawnPacket.RemoveAll(this);
		NetworkSubsystem->OnMovePacket.RemoveAll(this);
		NetworkSubsystem->OnChatPacket.RemoveAll(this);
		NetworkSubsystem->OnCombatActionPacket.RemoveAll(this);
		NetworkSubsystem->OnCombatActionResultPacket.RemoveAll(this);
		NetworkSubsystem->OnConnectionStateChanged.RemoveAll(this);
		NetworkSubsystem->OnMonsterUpdatePacket.RemoveAll(this);
		NetworkSubsystem->OnMonsterAttackPacket.RemoveAll(this);
		NetworkSubsystem->OnDamageResultPacket.RemoveAll(this);
		NetworkSubsystem->OnPlayerStatusPacket.RemoveAll(this);
		NetworkSubsystem->OnPlayerHealedPacket.RemoveAll(this);
		NetworkSubsystem->OnLootSnapshotPacket.RemoveAll(this);
		NetworkSubsystem->OnLootCreatedPacket.RemoveAll(this);
		NetworkSubsystem->OnLootRemovedPacket.RemoveAll(this);
	}

	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->OnLoginCompleted.RemoveDynamic(
			this,
			&US1GameInstance::HandleAccountLoginCompleted);
		AccountSubsystem->OnSignUpCompleted.RemoveDynamic(
			this,
			&US1GameInstance::HandleAccountSignUpCompleted);
		AccountSubsystem->OnCharacterCreated.RemoveDynamic(
			this,
			&US1GameInstance::HandleAccountCharacterCreated);
		AccountSubsystem->OnEnterGameCompleted.RemoveDynamic(
			this,
			&US1GameInstance::HandleAccountEnterGameCompleted);
	}

	if (AuthWidget != nullptr)
	{
		AuthWidget->RemoveFromParent();
		AuthWidget = nullptr;
	}

	if (CharacterSelectWidget != nullptr)
	{
		CharacterSelectWidget->RemoveFromParent();
		CharacterSelectWidget = nullptr;
	}

	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->ResetRegistry();
	}
	DisConnectFromGameServer();
	Super::Shutdown();
}

void US1GameInstance::EnterLobbyState()
{
	if(auto* Inventory=GetSubsystem<US1InventorySubsystem>())Inventory->ResetSession();
	if (US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		LevelFlowSubsystem->EnterLobbyState();
	}

	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->ResetRegistry();
	}
}

void US1GameInstance::BeginMainLevelTravel()
{
	US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>();
	if (LevelFlowSubsystem == nullptr ||
		LevelFlowSubsystem->GetLevelPhase() != ES1LevelPhase::Lobby)
	{
		return;
	}

	HideAuthUI();
	HideCharacterSelectUI();
	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->ResetRegistry();
	}
	LevelFlowSubsystem->BeginMainLevelTravel();
}

void US1GameInstance::OpenMainLevel()
{
	if (US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		LevelFlowSubsystem->OpenMainLevel();
	}
}

void US1GameInstance::CompleteMainLevelEntry()
{
	if (US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		if (US1WorldEntitySubsystem* WorldEntitySubsystem =
			GetWorldEntitySubsystem())
		{
			WorldEntitySubsystem->ConfigureOtherPlayerClass(
				OtherPlayerClass);
		}

		LevelFlowSubsystem->CompleteMainLevelEntry();
		FlushPendingSpawns();
		FlushLootPresentation();
	}
}

void US1GameInstance::FlushPendingSpawns()
{
	US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>();
	if (LevelFlowSubsystem == nullptr ||
		!LevelFlowSubsystem->IsMainLevelReady())
	{
		return;
	}

	bool bNeedsRetry = false;
	for (const FS1PendingEntitySpawn& Pending :
		LevelFlowSubsystem->GetPendingSpawnsOrdered())
	{
		if (SpawnPendingEntity(Pending))
			LevelFlowSubsystem->RemovePendingSpawn(Pending.ObjectId);
		else
			bNeedsRetry = true;
	}

	if (bNeedsRetry)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick(
				this,
				&US1GameInstance::FlushPendingSpawns);
		}
	}
}

void US1GameInstance::TravelToLobby()
{
	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->ResetRegistry();
	}

	if (US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		LevelFlowSubsystem->TravelToLobby();
	}
}

ES1LevelPhase US1GameInstance::GetLevelPhase() const
{
	if (const US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		return LevelFlowSubsystem->GetLevelPhase();
	}

	return ES1LevelPhase::Lobby;
}

US1WorldEntitySubsystem*
US1GameInstance::GetWorldEntitySubsystem() const
{
	UWorld* World = GetWorld();
	return World != nullptr
		? World->GetSubsystem<US1WorldEntitySubsystem>()
		: nullptr;
}

void US1GameInstance::ConnectToGameServer()
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		NetworkSubsystem->Connect();
	}
}

void US1GameInstance::DisConnectFromGameServer()
{
	if(auto* Inventory=GetSubsystem<US1InventorySubsystem>())Inventory->ResetSession();
	CombatContractVersion = 0;
	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->ResetAuthenticationState();
	}

	if (US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		NetworkSubsystem->Disconnect(TEXT("GameInstance requested disconnect"));
	}
	if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
		Entities->ResetCombatImpactState();
}

void US1GameInstance::HandleRecvPackets()
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		NetworkSubsystem->HandleRecvPackets();
	}
}

void US1GameInstance::HandleEnterGamePacket(
	const Protocol::S_ENTER_GAME& Packet)
{
	if (Packet.success())
	{
		auto* Flow=GetSubsystem<US1LevelFlowSubsystem>();
		if(!Flow||!FS1RegionContract::Supports(Packet)||!Flow->SetInitialRegion(Packet.region_id()))return;
		CombatContractVersion = Packet.combat_contract_version();
		if(auto* Inventory=GetSubsystem<US1InventorySubsystem>())
			Inventory->BeginSession(Packet.player().character_info().character_id(),Packet.loot_contract_version()==1,Packet.item_use_contract_version()==1,Packet.inventory_move_contract_version()==1,Packet.inventory_stack_contract_version()==1,Packet.player_field_drop_contract_version()==1);
		Flow->GetLootState().Reset(Packet.loot_presentation_version()==1);
		HandleSpawn(Packet);
		BeginMainLevelTravel();
	}
}

void US1GameInstance::HandleLeaveGamePacket(
	const Protocol::S_LEAVE_GAME& Packet)
{
	if(auto* Inventory=GetSubsystem<US1InventorySubsystem>())Inventory->ResetSession();
	// Level flow will consume this packet after it is separated.
}

void US1GameInstance::HandleSpawnPacket(
	const Protocol::S_SPAWN& Packet)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("S_SPAWN received. Count: %d"),
		Packet.players_size());

	for (const Protocol::ObjectInfo& Player : Packet.players())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"S_SPAWN player. ObjectId: %llu, "
				"Position: %.1f %.1f %.1f"),
			Player.object_id(),
			Player.pos_info().x(),
			Player.pos_info().y(),
			Player.pos_info().z());
	}

	HandleSpawn(Packet);
}

void US1GameInstance::HandleLootSnapshotPacket(const Protocol::S_LOOT_SNAPSHOT& Packet)
{
 if(!IsConnectedToGameServer())return;
 if(auto* Flow=GetSubsystem<US1LevelFlowSubsystem>())
  if(Flow->GetLootState().ApplySnapshot(Packet))FlushLootPresentation();
}
void US1GameInstance::HandleLootCreatedPacket(const Protocol::S_LOOT_CREATED& Packet)
{
 if(!IsConnectedToGameServer())return;
 if(auto* Flow=GetSubsystem<US1LevelFlowSubsystem>())
  if(Flow->GetLootState().ApplyCreated(Packet))FlushLootPresentation();
}
void US1GameInstance::HandleLootRemovedPacket(const Protocol::S_LOOT_REMOVED& Packet)
{
 if(!IsConnectedToGameServer())return;
 if(auto* Flow=GetSubsystem<US1LevelFlowSubsystem>())
  if(Flow->GetLootState().ApplyRemoved(Packet))FlushLootPresentation();
}
void US1GameInstance::FlushLootPresentation()
{
 auto* Flow=GetSubsystem<US1LevelFlowSubsystem>();
 if(!Flow||!Flow->IsMainLevelReady())return;
 if(auto* Entities=GetWorldEntitySubsystem())
  if(!Entities->SyncLootPresentation(Flow->GetLootState().GetDrops()))
   if(UWorld* World=GetWorld())World->GetTimerManager().SetTimerForNextTick(this,&US1GameInstance::FlushLootPresentation);
}

void US1GameInstance::HandleDespawnPacket(
	const Protocol::S_DESPAWN& Packet)
{
	HandleDeSpawn(Packet);
}

void US1GameInstance::HandleMovePacket(
	const Protocol::S_MOVE& Packet)
{
	HandleMove(Packet);
}

void US1GameInstance::HandleChatPacket(
	const Protocol::S_CHAT& Packet)
{
	// Chat presentation will subscribe independently later.
}

void US1GameInstance::HandleCombatActionPacket(
	const Protocol::S_COMBAT_ACTION& Packet)
{
	if (!IsConnectedToGameServer())
	{
		return;
	}

	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->ApplyCombatAction(Packet);
	}
}

void US1GameInstance::HandleCombatActionResultPacket(
	const Protocol::S_COMBAT_ACTION_RESULT& Packet)
{
	if (!IsConnectedToGameServer() || !SupportsCombatApproval())
		return;
	if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
		Entities->ApplyCombatActionResult(Packet);
}

void US1GameInstance::HandleCombatConnectionState(
	ES1ConnectionState State, const FString& Reason)
{
	if (State == ES1ConnectionState::Disconnected ||
		State == ES1ConnectionState::Disconnecting ||
		State == ES1ConnectionState::Error)
	{
		CombatContractVersion = 0;
		if(auto* Inventory=GetSubsystem<US1InventorySubsystem>())Inventory->ResetSession();
		if(auto* Flow=GetSubsystem<US1LevelFlowSubsystem>())Flow->EnterLobbyState();
		if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
		{
			Entities->ResetLocalCombatPrediction(true);
			Entities->ResetCombatImpactState();
			Entities->ResetLootRegistry();
		}
	}
}

void US1GameInstance::HandleMonsterUpdatePacket(const Protocol::S_MONSTER_UPDATE& Packet)
{
	if (!IsConnectedToGameServer()) return;
	if (US1LevelFlowSubsystem* Flow = GetSubsystem<US1LevelFlowSubsystem>())
	{
		if (!Flow->IsMainLevelReady())
		{
			Flow->UpdatePendingMonster(Packet);
			return;
		}
	}
	if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
	{
		Entities->ApplyMonsterUpdate(Packet);
	}
}

void US1GameInstance::HandleMonsterAttackPacket(const Protocol::S_MONSTER_ATTACK& Packet)
{
	if (!IsConnectedToGameServer()) return;
	if (US1LevelFlowSubsystem* Flow = GetSubsystem<US1LevelFlowSubsystem>())
	{
		if (!Flow->IsMainLevelReady()) return;
	}
	if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
	{
		Entities->ApplyMonsterAttack(Packet);
	}
}

void US1GameInstance::HandleDamageResultPacket(const Protocol::S_DAMAGE_RESULT& Packet)
{
	if (!IsConnectedToGameServer()) return;
	if (US1LevelFlowSubsystem* Flow = GetSubsystem<US1LevelFlowSubsystem>())
	{
		if (!Flow->IsMainLevelReady())
		{
			Flow->UpdatePendingDamage(Packet);
			return;
		}
	}
	if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
	{
		Entities->ApplyDamageResult(Packet);
	}
}

void US1GameInstance::HandlePlayerStatusPacket(const Protocol::S_PLAYER_STATUS& Packet)
{
	if (!IsConnectedToGameServer()) return;
	if (US1LevelFlowSubsystem* Flow = GetSubsystem<US1LevelFlowSubsystem>())
	{
		if (!Flow->IsMainLevelReady()) return;
	}
	if (US1WorldEntitySubsystem* Entities = GetWorldEntitySubsystem())
	{
		Entities->ApplyPlayerStatus(Packet);
	}
}

void US1GameInstance::HandlePlayerHealedPacket(const Protocol::S_PLAYER_HEALED& Packet)
{
 if(!IsConnectedToGameServer())return;
 auto* Flow=GetSubsystem<US1LevelFlowSubsystem>();
 if(!Flow||!Flow->IsMainLevelReady())return;
 if(auto* Entities=GetWorldEntitySubsystem())Entities->ApplyPlayerHealed(Packet);
}

void US1GameInstance::Login(
	const FString& AccountName,
	const FString& Password)
{
	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->Login(AccountName, Password);
	}
}

void US1GameInstance::LoginFromAuthUI(
	const FString& AccountName,
	const FString& Password)
{
	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->LoginFromAuthUI(AccountName, Password);
	}
}

void US1GameInstance::SignUp(
	const FString& AccountName,
	const FString& Password)
{
	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->SignUp(AccountName, Password);
	}
}

void US1GameInstance::ShowAuthUI()
{
	// A recreated PIE world must require a fresh login request. Any response
	// still queued from the previous world is treated as unsolicited.
	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->ResetAuthenticationState();
	}

	// PIE restart or world recreation can leave the previous lobby widget
	// attached to the viewport while the same GameInstance is still alive.
	if (CharacterSelectWidget != nullptr)
		CharacterSelectWidget->RemoveFromParent();

	if (AuthWidget == nullptr)
	{
		AuthWidget = CreateWidget<US1AuthWidget>(
			this,
			US1AuthWidget::StaticClass());
	}

	if (AuthWidget != nullptr && !AuthWidget->IsInViewport())
		AuthWidget->AddToViewport(100);

	if (APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeUIOnly InputMode;
		if (AuthWidget != nullptr)
			InputMode.SetWidgetToFocus(AuthWidget->GetInitialFocusTarget());
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}
}

void US1GameInstance::HideAuthUI()
{
	if (AuthWidget != nullptr)
		AuthWidget->RemoveFromParent();

	if (APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

void US1GameInstance::ShowCharacterSelectUI()
{
	HideAuthUI();

	if (CharacterSelectWidget == nullptr)
	{
		CharacterSelectWidget = CreateWidget<US1CharacterSelectWidget>(
			this,
			US1CharacterSelectWidget::StaticClass());
	}

	if (CharacterSelectWidget != nullptr)
	{
		CharacterSelectWidget->RefreshCharacterList();
		if (!CharacterSelectWidget->IsInViewport())
			CharacterSelectWidget->AddToViewport(100);
	}

	if (APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeUIOnly InputMode;
		if (CharacterSelectWidget != nullptr)
			InputMode.SetWidgetToFocus(CharacterSelectWidget->GetInitialFocusTarget());
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}
}

void US1GameInstance::HideCharacterSelectUI()
{
	if (CharacterSelectWidget != nullptr)
		CharacterSelectWidget->RemoveFromParent();

	if (APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
}

bool US1GameInstance::IsConnectedToGameServer() const
{
	if (const US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		return NetworkSubsystem->IsConnected();
	}

	return false;
}

void US1GameInstance::HandleAccountLoginCompleted(
	bool bSuccess,
	const FString& Message)
{
	OnLoginCompleted.Broadcast(bSuccess, Message);

	if (bSuccess)
		ShowCharacterSelectUI();
}

void US1GameInstance::HandleAccountSignUpCompleted(
	bool bSuccess,
	int32 ResultCode,
	const FString& Message)
{
	OnSignUpCompleted.Broadcast(bSuccess, ResultCode, Message);
}

void US1GameInstance::HandleAccountCharacterCreated(
	bool bSuccess,
	const FString& Message)
{
	OnCharacterCreated.Broadcast(bSuccess, Message);
}

void US1GameInstance::HandleAccountEnterGameCompleted(
	bool bSuccess,
	const FString& Message)
{
	OnEnterGameCompleted.Broadcast(bSuccess, Message);
	// Travel is owned by HandleEnterGamePacket after region validation/spawn queuing,
	// independent of multicast delegate registration order.
}

void US1GameInstance::CreateCharacter(
	const FString& CharacterName,
	int32 PlayerType)
{
	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->CreateCharacter(CharacterName, PlayerType);
	}
}

void US1GameInstance::EnterCharacter(
	int64 CharacterId)
{
	if (!IsConnectedToGameServer() ||
		CharacterId <= 0)
	{
		return;
	}

	if (const US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem();
		WorldEntitySubsystem != nullptr &&
		WorldEntitySubsystem->HasLocalPlayer())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"EnterCharacter ignored. "
				"A character is already in the world."));
		return;
	}

	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->EnterCharacter(CharacterId);
	}
}

void US1GameInstance::EnterFirstCharacter()
{
	if (const US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem();
		WorldEntitySubsystem != nullptr &&
		WorldEntitySubsystem->HasLocalPlayer())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"EnterFirstCharacter ignored. "
				"A character is already in the world."));
		return;
	}

	if (US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		AccountSubsystem->EnterFirstCharacter();
	}
}

const TArray<FS1CharacterSummary>& US1GameInstance::GetCharacters() const
{
	static const TArray<FS1CharacterSummary> EmptyCharacters;
	if (const US1AccountSubsystem* AccountSubsystem =
		GetSubsystem<US1AccountSubsystem>())
	{
		return AccountSubsystem->GetCharacters();
	}

	return EmptyCharacters;
}

void US1GameInstance::SendPacket(SendBufferRef SendBuffer)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetSubsystem<US1NetworkSubsystem>())
	{
		NetworkSubsystem->SendPacket(SendBuffer);
	}
}

bool US1GameInstance::SpawnPendingEntity(
	const FS1PendingEntitySpawn& PendingSpawn)
{
	US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem();
	if (WorldEntitySubsystem == nullptr)
		return false;

	if (WorldEntitySubsystem->ContainsEntity(PendingSpawn.ObjectId))
		return true;

	Protocol::ObjectInfo ObjectInfo;
	ObjectInfo.set_object_id(PendingSpawn.ObjectId);
	ObjectInfo.set_creature_type(static_cast<Protocol::CreatureType>(PendingSpawn.CreatureType));
	ObjectInfo.set_monster_definition_id(PendingSpawn.MonsterDefinitionId);
	ObjectInfo.set_current_hp(PendingSpawn.CurrentHp);
	ObjectInfo.set_max_hp(PendingSpawn.MaxHp);
	ObjectInfo.set_monster_state(static_cast<Protocol::MonsterState>(PendingSpawn.MonsterState));
	Protocol::PosInfo* PosInfo = ObjectInfo.mutable_pos_info();
	PosInfo->set_object_id(PendingSpawn.ObjectId);
	PosInfo->set_x(PendingSpawn.Location.X);
	PosInfo->set_y(PendingSpawn.Location.Y);
	PosInfo->set_z(PendingSpawn.Location.Z);
	PosInfo->set_yaw(PendingSpawn.Yaw);
	PosInfo->set_state(
		static_cast<Protocol::MoveState>(PendingSpawn.MoveState));

	return PendingSpawn.CreatureType == Protocol::CREATURE_TYPE_MONSTER
		? WorldEntitySubsystem->SpawnMonster(ObjectInfo)
		: WorldEntitySubsystem->SpawnPlayer(ObjectInfo, PendingSpawn.bIsMine);
}

void US1GameInstance::HandleSpawn(const Protocol::ObjectInfo& ObjectInfo, bool IsMine)
{
	if (!IsConnectedToGameServer())
		return;

	US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>();
	if (LevelFlowSubsystem == nullptr)
		return;

	if (!LevelFlowSubsystem->IsMainLevelReady())
	{
		LevelFlowSubsystem->QueuePendingSpawn(ObjectInfo, IsMine);
		return;
	}

	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		if (ObjectInfo.creature_type() == Protocol::CREATURE_TYPE_MONSTER)
		{
			WorldEntitySubsystem->SpawnMonster(ObjectInfo);
		}
		else
		{
			WorldEntitySubsystem->SpawnPlayer(ObjectInfo, IsMine);
		}
	}
}

void US1GameInstance::HandleSpawn(const Protocol::S_ENTER_GAME& EnterGamePkt)
{
	HandleSpawn(EnterGamePkt.player(), true);
}

void US1GameInstance::HandleSpawn(const Protocol::S_SPAWN& SpawnPkt)
{
	for (auto& Player : SpawnPkt.players())
	{
		HandleSpawn(Player, false);
	}
	for (const auto& Monster : SpawnPkt.monsters())
	{
		HandleSpawn(Monster, false);
	}
}

void US1GameInstance::HandleDeSpawn(uint64 ObjectId)
{
	if (!IsConnectedToGameServer())
		return;

	if (US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		LevelFlowSubsystem->RemovePendingSpawn(ObjectId);
	}

	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->DespawnEntity(ObjectId);
	}
}

void US1GameInstance::HandleDeSpawn(const Protocol::S_DESPAWN& DeSpawnPkt)
{
	for (auto& Player : DeSpawnPkt.object_ids())
	{
		HandleDeSpawn(Player);
	}
}

void US1GameInstance::HandleMove(const Protocol::S_MOVE& MovePkt)
{
	if (!IsConnectedToGameServer())
		return;

	if (US1LevelFlowSubsystem* LevelFlowSubsystem =
		GetSubsystem<US1LevelFlowSubsystem>())
	{
		if (LevelFlowSubsystem->UpdatePendingMove(MovePkt))
			return;
	}

	if (US1WorldEntitySubsystem* WorldEntitySubsystem =
		GetWorldEntitySubsystem())
	{
		WorldEntitySubsystem->ApplyMove(MovePkt);
	}
}
