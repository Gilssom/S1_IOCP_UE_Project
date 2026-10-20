#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "S1.h"
#include "S1NetworkSubsystem.generated.h"

class FSocket;
class PacketSession;
namespace Protocol
{
	class S_LOGIN;
	class S_SIGN_UP;
	class S_CREATE_CHARACTER;
	class S_ENTER_GAME;
	class S_LEAVE_GAME;
	class S_SPAWN;
	class S_DESPAWN;
	class S_MOVE;
	class S_CHAT;
	class S_COMBAT_ACTION;
	class S_COMBAT_ACTION_RESULT;
	class S_MONSTER_UPDATE;
	class S_MONSTER_ATTACK;
	class S_DAMAGE_RESULT;
	class S_PLAYER_STATUS;
	class S_LOOT_SNAPSHOT;
	class S_LOOT_CREATED;
	class S_LOOT_REMOVED;
	class S_ITEM_PICKUP_RESULT;
	class S_INVENTORY_SNAPSHOT;
	class S_ITEM_USE_RESULT;
	class S_PLAYER_HEALED;
	class S_INVENTORY_MOVE_RESULT;
	class S_INVENTORY_STACK_ACTION_RESULT;
	class S_INVENTORY_DISCARD_RESULT;
	class S_PLAYER_FIELD_DROP_RESULT;
	class S_QUEST_STATE;
	class S_QUEST_ACCEPT_RESULT;
	class S_QUEST_TURN_IN_RESULT;
}

UENUM(BlueprintType)
enum class ES1ConnectionState : uint8
{
	Disconnected,
	Connecting,
	Connected,
	Disconnecting,
	Error
};

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnS1ConnectionStateChanged,
	ES1ConnectionState,
	const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1LoginPacket,
	const Protocol::S_LOGIN&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1SignUpPacket,
	const Protocol::S_SIGN_UP&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1CharacterCreatedPacket,
	const Protocol::S_CREATE_CHARACTER&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1EnterGamePacket,
	const Protocol::S_ENTER_GAME&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1LeaveGamePacket,
	const Protocol::S_LEAVE_GAME&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1SpawnPacket,
	const Protocol::S_SPAWN&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1DespawnPacket,
	const Protocol::S_DESPAWN&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1MovePacket,
	const Protocol::S_MOVE&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1ChatPacket,
	const Protocol::S_CHAT&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1CombatActionPacket,
	const Protocol::S_COMBAT_ACTION&);
DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnS1CombatActionResultPacket,
	const Protocol::S_COMBAT_ACTION_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1MonsterUpdatePacket, const Protocol::S_MONSTER_UPDATE&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1MonsterAttackPacket, const Protocol::S_MONSTER_ATTACK&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1DamageResultPacket, const Protocol::S_DAMAGE_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1PlayerStatusPacket, const Protocol::S_PLAYER_STATUS&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1LootSnapshotPacket, const Protocol::S_LOOT_SNAPSHOT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1LootCreatedPacket, const Protocol::S_LOOT_CREATED&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1LootRemovedPacket, const Protocol::S_LOOT_REMOVED&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1ItemPickupResultPacket, const Protocol::S_ITEM_PICKUP_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1InventorySnapshotPacket, const Protocol::S_INVENTORY_SNAPSHOT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1ItemUseResultPacket, const Protocol::S_ITEM_USE_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1PlayerHealedPacket, const Protocol::S_PLAYER_HEALED&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1InventoryMoveResultPacket, const Protocol::S_INVENTORY_MOVE_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1InventoryStackResultPacket, const Protocol::S_INVENTORY_STACK_ACTION_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1InventoryDiscardResultPacket, const Protocol::S_INVENTORY_DISCARD_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1PlayerFieldDropResultPacket, const Protocol::S_PLAYER_FIELD_DROP_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1QuestStatePacket, const Protocol::S_QUEST_STATE&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1QuestAcceptPacket, const Protocol::S_QUEST_ACCEPT_RESULT&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnS1QuestTurnInPacket, const Protocol::S_QUEST_TURN_IN_RESULT&);

UCLASS()
class S1_API US1NetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(
		FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool Connect();
	bool Connect(const FString& Host, int32 ServerPort);
	void Disconnect(const FString& Reason = TEXT("Requested"));

	bool IsConnected() const;
	FString GetServerProfileKey() const { return FString::Printf(TEXT("%s:%d"), *IpAddress, Port); }
	ES1ConnectionState GetConnectionState() const
	{
		return ConnectionState;
	}

	void SendPacket(SendBufferRef SendBuffer);
	void HandleRecvPackets();

	void NotifyLoginPacket(const Protocol::S_LOGIN& Packet);
	void NotifySignUpPacket(const Protocol::S_SIGN_UP& Packet);
	void NotifyCharacterCreatedPacket(
		const Protocol::S_CREATE_CHARACTER& Packet);
	void NotifyEnterGamePacket(const Protocol::S_ENTER_GAME& Packet);
	void NotifyLeaveGamePacket(const Protocol::S_LEAVE_GAME& Packet);
	void NotifySpawnPacket(const Protocol::S_SPAWN& Packet);
	void NotifyDespawnPacket(const Protocol::S_DESPAWN& Packet);
	void NotifyMovePacket(const Protocol::S_MOVE& Packet);
	void NotifyChatPacket(const Protocol::S_CHAT& Packet);
	void NotifyCombatActionPacket(const Protocol::S_COMBAT_ACTION& Packet);
	void NotifyCombatActionResultPacket(const Protocol::S_COMBAT_ACTION_RESULT& Packet);
	void NotifyMonsterUpdatePacket(const Protocol::S_MONSTER_UPDATE& Packet);
	void NotifyMonsterAttackPacket(const Protocol::S_MONSTER_ATTACK& Packet);
	void NotifyDamageResultPacket(const Protocol::S_DAMAGE_RESULT& Packet);
	void NotifyPlayerStatusPacket(const Protocol::S_PLAYER_STATUS& Packet);
	void NotifyLootSnapshotPacket(const Protocol::S_LOOT_SNAPSHOT& Packet);
	void NotifyLootCreatedPacket(const Protocol::S_LOOT_CREATED& Packet);
	void NotifyLootRemovedPacket(const Protocol::S_LOOT_REMOVED& Packet);
	void NotifyItemPickupResultPacket(const Protocol::S_ITEM_PICKUP_RESULT& Packet);
	void NotifyInventorySnapshotPacket(const Protocol::S_INVENTORY_SNAPSHOT& Packet);
	void NotifyItemUseResultPacket(const Protocol::S_ITEM_USE_RESULT& Packet);
	void NotifyPlayerHealedPacket(const Protocol::S_PLAYER_HEALED& Packet);
	void NotifyInventoryMoveResultPacket(const Protocol::S_INVENTORY_MOVE_RESULT& Packet);
	void NotifyInventoryStackResultPacket(const Protocol::S_INVENTORY_STACK_ACTION_RESULT& Packet);
	void NotifyInventoryDiscardResultPacket(const Protocol::S_INVENTORY_DISCARD_RESULT& Packet);
	void NotifyPlayerFieldDropResultPacket(const Protocol::S_PLAYER_FIELD_DROP_RESULT& Packet);
	void NotifyQuestStatePacket(const Protocol::S_QUEST_STATE& Packet);
	void NotifyQuestAcceptPacket(const Protocol::S_QUEST_ACCEPT_RESULT& Packet);
	void NotifyQuestTurnInPacket(const Protocol::S_QUEST_TURN_IN_RESULT& Packet);

	FOnS1ConnectionStateChanged OnConnectionStateChanged;
	FOnS1LoginPacket OnLoginPacket;
	FOnS1SignUpPacket OnSignUpPacket;
	FOnS1CharacterCreatedPacket OnCharacterCreatedPacket;
	FOnS1EnterGamePacket OnEnterGamePacket;
	FOnS1LeaveGamePacket OnLeaveGamePacket;
	FOnS1SpawnPacket OnSpawnPacket;
	FOnS1DespawnPacket OnDespawnPacket;
	FOnS1MovePacket OnMovePacket;
	FOnS1ChatPacket OnChatPacket;
	FOnS1CombatActionPacket OnCombatActionPacket;
	FOnS1CombatActionResultPacket OnCombatActionResultPacket;
	FOnS1MonsterUpdatePacket OnMonsterUpdatePacket;
	FOnS1MonsterAttackPacket OnMonsterAttackPacket;
	FOnS1DamageResultPacket OnDamageResultPacket;
	FOnS1PlayerStatusPacket OnPlayerStatusPacket;
	FOnS1LootSnapshotPacket OnLootSnapshotPacket;
	FOnS1LootCreatedPacket OnLootCreatedPacket;
	FOnS1LootRemovedPacket OnLootRemovedPacket;
	FOnS1ItemPickupResultPacket OnItemPickupResultPacket;
	FOnS1InventorySnapshotPacket OnInventorySnapshotPacket;
	FOnS1ItemUseResultPacket OnItemUseResultPacket;
	FOnS1PlayerHealedPacket OnPlayerHealedPacket;
	FOnS1InventoryMoveResultPacket OnInventoryMoveResultPacket;
	FOnS1InventoryStackResultPacket OnInventoryStackResultPacket;
	FOnS1InventoryDiscardResultPacket OnInventoryDiscardResultPacket;
	FOnS1PlayerFieldDropResultPacket OnPlayerFieldDropResultPacket;
	FOnS1QuestStatePacket OnQuestStatePacket;
	FOnS1QuestAcceptPacket OnQuestAcceptPacket;
	FOnS1QuestTurnInPacket OnQuestTurnInPacket;

private:
	bool TickNetwork(float DeltaTime);
	void DestroySocket();
	void SetConnectionState(
		ES1ConnectionState NewState,
		const FString& Reason);

private:
	FSocket* Socket = nullptr;
	TSharedPtr<PacketSession> GameServerSession;
	FTSTicker::FDelegateHandle NetworkTickerHandle;
	ES1ConnectionState ConnectionState =
		ES1ConnectionState::Disconnected;

	FString IpAddress = TEXT("127.0.0.1");
	int32 Port = 7777;
};
