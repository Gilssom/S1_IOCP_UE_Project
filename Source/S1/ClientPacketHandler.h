#pragma once
#include "BufferReader.h"
#include "BufferWriter.h"
#include "Protocol.pb.h"

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "S1.h"
#endif

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_SIGN_UP = 1002,
	PKT_S_SIGN_UP = 1003,
	PKT_C_ENTER_GAME = 1004,
	PKT_S_ENTER_GAME = 1005,
	PKT_C_LEAVE_GAME = 1006,
	PKT_S_LEAVE_GAME = 1007,
	PKT_S_SPAWN = 1008,
	PKT_S_DESPAWN = 1009,
	PKT_C_MOVE = 1010,
	PKT_S_MOVE = 1011,
	PKT_C_CHAT = 1012,
	PKT_S_CHAT = 1013,
	PKT_C_CREATE_CHARACTER = 1014,
	PKT_S_CREATE_CHARACTER = 1015,
	PKT_C_COMBAT_ACTION = 1016,
	PKT_S_COMBAT_ACTION = 1017,
	PKT_S_MONSTER_UPDATE = 1018,
	PKT_S_MONSTER_ATTACK = 1019,
	PKT_S_DAMAGE_RESULT = 1020,
	PKT_S_PLAYER_STATUS = 1021,
	PKT_S_COMBAT_ACTION_RESULT = 1022,
	PKT_S_LOOT_SNAPSHOT = 1023,
	PKT_S_LOOT_CREATED = 1024,
	PKT_S_LOOT_REMOVED = 1025,
	PKT_C_ITEM_PICKUP = 1026,
	PKT_S_ITEM_PICKUP_RESULT = 1027,
	PKT_C_INVENTORY_QUERY = 1028,
	PKT_S_INVENTORY_SNAPSHOT = 1029,
	PKT_C_ITEM_USE = 1030,
	PKT_S_ITEM_USE_RESULT = 1031,
	PKT_S_PLAYER_HEALED = 1032,
	PKT_C_INVENTORY_MOVE = 1033,
	PKT_S_INVENTORY_MOVE_RESULT = 1034,
	PKT_C_INVENTORY_STACK_ACTION = 1035,
	PKT_S_INVENTORY_STACK_ACTION_RESULT = 1036,
	PKT_C_INVENTORY_DISCARD = 1037,
	PKT_S_INVENTORY_DISCARD_RESULT = 1038,
	PKT_C_PLAYER_FIELD_DROP = 1039,
	PKT_S_PLAYER_FIELD_DROP_RESULT = 1040,
	PKT_C_QUEST_QUERY = 1041,
	PKT_S_QUEST_STATE = 1042,
	PKT_C_QUEST_ACCEPT = 1043,
	PKT_S_QUEST_ACCEPT_RESULT = 1044,
	PKT_C_QUEST_TURN_IN = 1045,
	PKT_S_QUEST_TURN_IN_RESULT = 1046,
	PKT_C_PORTAL_TRANSFER_REQUEST = 1047,
	PKT_S_PORTAL_TRANSFER_PREPARE = 1048,
	PKT_C_PORTAL_TRANSFER_READY = 1049,
	PKT_S_PORTAL_TRANSFER_COMMIT = 1050,
	PKT_C_PORTAL_WORLD_READY = 1051,
};

// Custom Handler
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_SIGN_UP(PacketSessionRef& session, Protocol::S_SIGN_UP& pkt);
bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt);
bool Handle_S_LEAVE_GAME(PacketSessionRef& session, Protocol::S_LEAVE_GAME& pkt);
bool Handle_S_SPAWN(PacketSessionRef& session, Protocol::S_SPAWN& pkt);
bool Handle_S_DESPAWN(PacketSessionRef& session, Protocol::S_DESPAWN& pkt);
bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt);
bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt);
bool Handle_S_CREATE_CHARACTER(PacketSessionRef& session, Protocol::S_CREATE_CHARACTER& pkt);
bool Handle_S_COMBAT_ACTION(PacketSessionRef& session, Protocol::S_COMBAT_ACTION& pkt);
bool Handle_S_MONSTER_UPDATE(PacketSessionRef& session, Protocol::S_MONSTER_UPDATE& pkt);
bool Handle_S_MONSTER_ATTACK(PacketSessionRef& session, Protocol::S_MONSTER_ATTACK& pkt);
bool Handle_S_DAMAGE_RESULT(PacketSessionRef& session, Protocol::S_DAMAGE_RESULT& pkt);
bool Handle_S_PLAYER_STATUS(PacketSessionRef& session, Protocol::S_PLAYER_STATUS& pkt);
bool Handle_S_COMBAT_ACTION_RESULT(PacketSessionRef& session, Protocol::S_COMBAT_ACTION_RESULT& pkt);
bool Handle_S_LOOT_SNAPSHOT(PacketSessionRef& session, Protocol::S_LOOT_SNAPSHOT& pkt);
bool Handle_S_LOOT_CREATED(PacketSessionRef& session, Protocol::S_LOOT_CREATED& pkt);
bool Handle_S_LOOT_REMOVED(PacketSessionRef& session, Protocol::S_LOOT_REMOVED& pkt);
bool Handle_S_ITEM_PICKUP_RESULT(PacketSessionRef& session, Protocol::S_ITEM_PICKUP_RESULT& pkt);
bool Handle_S_INVENTORY_SNAPSHOT(PacketSessionRef& session, Protocol::S_INVENTORY_SNAPSHOT& pkt);
bool Handle_S_ITEM_USE_RESULT(PacketSessionRef& session, Protocol::S_ITEM_USE_RESULT& pkt);
bool Handle_S_PLAYER_HEALED(PacketSessionRef& session, Protocol::S_PLAYER_HEALED& pkt);
bool Handle_S_INVENTORY_MOVE_RESULT(PacketSessionRef& session, Protocol::S_INVENTORY_MOVE_RESULT& pkt);
bool Handle_S_INVENTORY_STACK_ACTION_RESULT(PacketSessionRef& session, Protocol::S_INVENTORY_STACK_ACTION_RESULT& pkt);
bool Handle_S_INVENTORY_DISCARD_RESULT(PacketSessionRef& session, Protocol::S_INVENTORY_DISCARD_RESULT& pkt);
bool Handle_S_PLAYER_FIELD_DROP_RESULT(PacketSessionRef& session, Protocol::S_PLAYER_FIELD_DROP_RESULT& pkt);
bool Handle_S_QUEST_STATE(PacketSessionRef& session, Protocol::S_QUEST_STATE& pkt);
bool Handle_S_QUEST_ACCEPT_RESULT(PacketSessionRef& session, Protocol::S_QUEST_ACCEPT_RESULT& pkt);
bool Handle_S_QUEST_TURN_IN_RESULT(PacketSessionRef& session, Protocol::S_QUEST_TURN_IN_RESULT& pkt);
bool Handle_S_PORTAL_TRANSFER_PREPARE(PacketSessionRef& session, Protocol::S_PORTAL_TRANSFER_PREPARE& pkt);
bool Handle_S_PORTAL_TRANSFER_COMMIT(PacketSessionRef& session, Protocol::S_PORTAL_TRANSFER_COMMIT& pkt);

class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int32 i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len); };
		GPacketHandler[PKT_S_SIGN_UP] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_SIGN_UP>(Handle_S_SIGN_UP, session, buffer, len); };
		GPacketHandler[PKT_S_ENTER_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_ENTER_GAME>(Handle_S_ENTER_GAME, session, buffer, len); };
		GPacketHandler[PKT_S_LEAVE_GAME] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_LEAVE_GAME>(Handle_S_LEAVE_GAME, session, buffer, len); };
		GPacketHandler[PKT_S_SPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_SPAWN>(Handle_S_SPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_DESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_DESPAWN>(Handle_S_DESPAWN, session, buffer, len); };
		GPacketHandler[PKT_S_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_MOVE>(Handle_S_MOVE, session, buffer, len); };
		GPacketHandler[PKT_S_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_CHAT>(Handle_S_CHAT, session, buffer, len); };
		GPacketHandler[PKT_S_CREATE_CHARACTER] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_CREATE_CHARACTER>(Handle_S_CREATE_CHARACTER, session, buffer, len); };
		GPacketHandler[PKT_S_COMBAT_ACTION] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_COMBAT_ACTION>(Handle_S_COMBAT_ACTION, session, buffer, len); };
		GPacketHandler[PKT_S_MONSTER_UPDATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_MONSTER_UPDATE>(Handle_S_MONSTER_UPDATE, session, buffer, len); };
		GPacketHandler[PKT_S_MONSTER_ATTACK] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_MONSTER_ATTACK>(Handle_S_MONSTER_ATTACK, session, buffer, len); };
		GPacketHandler[PKT_S_DAMAGE_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_DAMAGE_RESULT>(Handle_S_DAMAGE_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_PLAYER_STATUS] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_PLAYER_STATUS>(Handle_S_PLAYER_STATUS, session, buffer, len); };
		GPacketHandler[PKT_S_COMBAT_ACTION_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_COMBAT_ACTION_RESULT>(Handle_S_COMBAT_ACTION_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_LOOT_SNAPSHOT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_LOOT_SNAPSHOT>(Handle_S_LOOT_SNAPSHOT, session, buffer, len); };
		GPacketHandler[PKT_S_LOOT_CREATED] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_LOOT_CREATED>(Handle_S_LOOT_CREATED, session, buffer, len); };
		GPacketHandler[PKT_S_LOOT_REMOVED] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_LOOT_REMOVED>(Handle_S_LOOT_REMOVED, session, buffer, len); };
		GPacketHandler[PKT_S_ITEM_PICKUP_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_ITEM_PICKUP_RESULT>(Handle_S_ITEM_PICKUP_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_INVENTORY_SNAPSHOT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_INVENTORY_SNAPSHOT>(Handle_S_INVENTORY_SNAPSHOT, session, buffer, len); };
		GPacketHandler[PKT_S_ITEM_USE_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_ITEM_USE_RESULT>(Handle_S_ITEM_USE_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_PLAYER_HEALED] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_PLAYER_HEALED>(Handle_S_PLAYER_HEALED, session, buffer, len); };
		GPacketHandler[PKT_S_INVENTORY_MOVE_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_INVENTORY_MOVE_RESULT>(Handle_S_INVENTORY_MOVE_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_INVENTORY_STACK_ACTION_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_INVENTORY_STACK_ACTION_RESULT>(Handle_S_INVENTORY_STACK_ACTION_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_INVENTORY_DISCARD_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_INVENTORY_DISCARD_RESULT>(Handle_S_INVENTORY_DISCARD_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_PLAYER_FIELD_DROP_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_PLAYER_FIELD_DROP_RESULT>(Handle_S_PLAYER_FIELD_DROP_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_QUEST_STATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_QUEST_STATE>(Handle_S_QUEST_STATE, session, buffer, len); };
		GPacketHandler[PKT_S_QUEST_ACCEPT_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_QUEST_ACCEPT_RESULT>(Handle_S_QUEST_ACCEPT_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_QUEST_TURN_IN_RESULT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_QUEST_TURN_IN_RESULT>(Handle_S_QUEST_TURN_IN_RESULT, session, buffer, len); };
		GPacketHandler[PKT_S_PORTAL_TRANSFER_PREPARE] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_PORTAL_TRANSFER_PREPARE>(Handle_S_PORTAL_TRANSFER_PREPARE, session, buffer, len); };
		GPacketHandler[PKT_S_PORTAL_TRANSFER_COMMIT] = [](PacketSessionRef& session, BYTE* buffer, int32 len){ return HandlePacket<Protocol::S_PORTAL_TRANSFER_COMMIT>(Handle_S_PORTAL_TRANSFER_COMMIT, session, buffer, len); };
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_SIGN_UP& pkt) { return MakeSendBuffer(pkt, PKT_C_SIGN_UP); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ENTER_GAME& pkt) { return MakeSendBuffer(pkt, PKT_C_ENTER_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::C_LEAVE_GAME& pkt) { return MakeSendBuffer(pkt, PKT_C_LEAVE_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CREATE_CHARACTER& pkt) { return MakeSendBuffer(pkt, PKT_C_CREATE_CHARACTER); }
	static SendBufferRef MakeSendBuffer(Protocol::C_COMBAT_ACTION& pkt) { return MakeSendBuffer(pkt, PKT_C_COMBAT_ACTION); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ITEM_PICKUP& pkt) { return MakeSendBuffer(pkt, PKT_C_ITEM_PICKUP); }
	static SendBufferRef MakeSendBuffer(Protocol::C_INVENTORY_QUERY& pkt) { return MakeSendBuffer(pkt, PKT_C_INVENTORY_QUERY); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ITEM_USE& pkt) { return MakeSendBuffer(pkt, PKT_C_ITEM_USE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_INVENTORY_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_INVENTORY_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_INVENTORY_STACK_ACTION& pkt) { return MakeSendBuffer(pkt, PKT_C_INVENTORY_STACK_ACTION); }
	static SendBufferRef MakeSendBuffer(Protocol::C_INVENTORY_DISCARD& pkt) { return MakeSendBuffer(pkt, PKT_C_INVENTORY_DISCARD); }
	static SendBufferRef MakeSendBuffer(Protocol::C_PLAYER_FIELD_DROP& pkt) { return MakeSendBuffer(pkt, PKT_C_PLAYER_FIELD_DROP); }
	static SendBufferRef MakeSendBuffer(Protocol::C_QUEST_QUERY& pkt) { return MakeSendBuffer(pkt, PKT_C_QUEST_QUERY); }
	static SendBufferRef MakeSendBuffer(Protocol::C_QUEST_ACCEPT& pkt) { return MakeSendBuffer(pkt, PKT_C_QUEST_ACCEPT); }
	static SendBufferRef MakeSendBuffer(Protocol::C_QUEST_TURN_IN& pkt) { return MakeSendBuffer(pkt, PKT_C_QUEST_TURN_IN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_PORTAL_TRANSFER_REQUEST& pkt) { return MakeSendBuffer(pkt, PKT_C_PORTAL_TRANSFER_REQUEST); }
	static SendBufferRef MakeSendBuffer(Protocol::C_PORTAL_TRANSFER_READY& pkt) { return MakeSendBuffer(pkt, PKT_C_PORTAL_TRANSFER_READY); }
	static SendBufferRef MakeSendBuffer(Protocol::C_PORTAL_WORLD_READY& pkt) { return MakeSendBuffer(pkt, PKT_C_PORTAL_WORLD_READY); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(packetSize);
#else
		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
#endif

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		pkt.SerializeToArray(&header[1], dataSize);
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};