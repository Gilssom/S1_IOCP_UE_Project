#include "ClientPacketHandler.h"

#include "Network/PacketSession.h"
#include "Network/S1NetworkSubsystem.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

namespace
{
	US1NetworkSubsystem* GetNetworkSubsystem(
		const PacketSessionRef& Session)
	{
		return Session != nullptr
			? Session->GetOwnerNetworkSubsystem()
			: nullptr;
	}
}

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

bool Handle_S_LOGIN(
	PacketSessionRef& session,
	Protocol::S_LOGIN& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyLoginPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_SIGN_UP(
	PacketSessionRef& session,
	Protocol::S_SIGN_UP& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifySignUpPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_CREATE_CHARACTER(
	PacketSessionRef& session,
	Protocol::S_CREATE_CHARACTER& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyCharacterCreatedPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_ENTER_GAME(
	PacketSessionRef& session,
	Protocol::S_ENTER_GAME& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyEnterGamePacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_LEAVE_GAME(
	PacketSessionRef& session,
	Protocol::S_LEAVE_GAME& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyLeaveGamePacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_SPAWN(
	PacketSessionRef& session,
	Protocol::S_SPAWN& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifySpawnPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_DESPAWN(
	PacketSessionRef& session,
	Protocol::S_DESPAWN& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyDespawnPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_MOVE(
	PacketSessionRef& session,
	Protocol::S_MOVE& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyMovePacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_CHAT(
	PacketSessionRef& session,
	Protocol::S_CHAT& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyChatPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_COMBAT_ACTION(
	PacketSessionRef& session,
	Protocol::S_COMBAT_ACTION& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem =
		GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyCombatActionPacket(pkt);
		return true;
	}

	return false;
}

bool Handle_S_COMBAT_ACTION_RESULT(
	PacketSessionRef& session,
	Protocol::S_COMBAT_ACTION_RESULT& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyCombatActionResultPacket(pkt);
		return true;
	}
	return false;
}

bool Handle_S_MONSTER_UPDATE(PacketSessionRef& session, Protocol::S_MONSTER_UPDATE& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyMonsterUpdatePacket(pkt);
		return true;
	}
	return false;
}

bool Handle_S_MONSTER_ATTACK(PacketSessionRef& session, Protocol::S_MONSTER_ATTACK& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyMonsterAttackPacket(pkt);
		return true;
	}
	return false;
}

bool Handle_S_DAMAGE_RESULT(PacketSessionRef& session, Protocol::S_DAMAGE_RESULT& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyDamageResultPacket(pkt);
		return true;
	}
	return false;
}

bool Handle_S_PLAYER_STATUS(PacketSessionRef& session, Protocol::S_PLAYER_STATUS& pkt)
{
	if (US1NetworkSubsystem* NetworkSubsystem = GetNetworkSubsystem(session))
	{
		NetworkSubsystem->NotifyPlayerStatusPacket(pkt);
		return true;
	}
	return false;
}

bool Handle_S_LOOT_SNAPSHOT(PacketSessionRef& session, Protocol::S_LOOT_SNAPSHOT& pkt)
{
 if (auto* Network = GetNetworkSubsystem(session)) { Network->NotifyLootSnapshotPacket(pkt); return true; }
 return false;
}
bool Handle_S_LOOT_CREATED(PacketSessionRef& session, Protocol::S_LOOT_CREATED& pkt)
{
 if (auto* Network = GetNetworkSubsystem(session)) { Network->NotifyLootCreatedPacket(pkt); return true; }
 return false;
}
bool Handle_S_LOOT_REMOVED(PacketSessionRef& session, Protocol::S_LOOT_REMOVED& pkt)
{
 if (auto* Network = GetNetworkSubsystem(session)) { Network->NotifyLootRemovedPacket(pkt); return true; }
 return false;
}
bool Handle_S_ITEM_PICKUP_RESULT(PacketSessionRef& session, Protocol::S_ITEM_PICKUP_RESULT& pkt)
{
 if (auto* Network = GetNetworkSubsystem(session)) { Network->NotifyItemPickupResultPacket(pkt); return true; }
 return false;
}
bool Handle_S_INVENTORY_SNAPSHOT(PacketSessionRef& session, Protocol::S_INVENTORY_SNAPSHOT& pkt)
{
 if (auto* Network = GetNetworkSubsystem(session)) { Network->NotifyInventorySnapshotPacket(pkt); return true; }
 return false;
}
bool Handle_S_ITEM_USE_RESULT(PacketSessionRef& session,Protocol::S_ITEM_USE_RESULT& pkt)
{if(auto* Network=GetNetworkSubsystem(session)){Network->NotifyItemUseResultPacket(pkt);return true;}return false;}
bool Handle_S_PLAYER_HEALED(PacketSessionRef& session,Protocol::S_PLAYER_HEALED& pkt)
{if(auto* Network=GetNetworkSubsystem(session)){Network->NotifyPlayerHealedPacket(pkt);return true;}return false;}
