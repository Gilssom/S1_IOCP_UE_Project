#include "Network/S1NetworkSubsystem.h"

#include "Interfaces/IPv4/IPv4Address.h"
#include "Network/PacketSession.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

void US1NetworkSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	NetworkTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(
			this,
			&US1NetworkSubsystem::TickNetwork));
}

void US1NetworkSubsystem::Deinitialize()
{
	if (NetworkTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(NetworkTickerHandle);
		NetworkTickerHandle.Reset();
	}

	Disconnect(TEXT("Network subsystem deinitialized"));
	Super::Deinitialize();
}

bool US1NetworkSubsystem::Connect()
{
	return Connect(IpAddress, Port);
}

bool US1NetworkSubsystem::Connect(
	const FString& Host,
	int32 ServerPort)
{
	if (ConnectionState == ES1ConnectionState::Connecting ||
		ConnectionState == ES1ConnectionState::Connected ||
		ConnectionState == ES1ConnectionState::Disconnecting)
	{
		return false;
	}

	if (Socket != nullptr || GameServerSession != nullptr)
		Disconnect(TEXT("Cleaning stale connection before reconnect"));

	SetConnectionState(
		ES1ConnectionState::Connecting,
		TEXT("Connecting to game server"));

	ISocketSubsystem* SocketSubsystem =
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSubsystem == nullptr)
	{
		SetConnectionState(
			ES1ConnectionState::Error,
			TEXT("Socket subsystem is unavailable"));
		return false;
	}

	Socket = SocketSubsystem->CreateSocket(
		TEXT("Stream"),
		TEXT("S1 Game Server Socket"));
	if (Socket == nullptr)
	{
		SetConnectionState(
			ES1ConnectionState::Error,
			TEXT("Socket creation failed"));
		return false;
	}

	FIPv4Address ParsedAddress;
	if (!FIPv4Address::Parse(Host, ParsedAddress))
	{
		DestroySocket();
		SetConnectionState(
			ES1ConnectionState::Error,
			TEXT("Invalid server address"));
		return false;
	}

	TSharedPtr<FInternetAddr> InternetAddress =
		SocketSubsystem->CreateInternetAddr();
	InternetAddress->SetIp(ParsedAddress.Value);
	InternetAddress->SetPort(ServerPort);

	if (!Socket->Connect(*InternetAddress))
	{
		DestroySocket();
		SetConnectionState(
			ES1ConnectionState::Error,
			TEXT("Game server connection failed"));
		UE_LOG(
			LogTemp,
			Error,
			TEXT("Game server connection failed. Host[%s], Port[%d]"),
			*Host,
			ServerPort);
		return false;
	}

	IpAddress = Host;
	Port = ServerPort;
	GameServerSession = MakeShared<PacketSession>(Socket, this);
	GameServerSession->Run();
	SetConnectionState(
		ES1ConnectionState::Connected,
		TEXT("Game server connected"));

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Game server connected. Host[%s], Port[%d]"),
		*Host,
		ServerPort);
	return true;
}

void US1NetworkSubsystem::Disconnect(const FString& Reason)
{
	if (ConnectionState == ES1ConnectionState::Disconnected &&
		Socket == nullptr &&
		GameServerSession == nullptr)
	{
		return;
	}

	SetConnectionState(
		ES1ConnectionState::Disconnecting,
		Reason);

	if (GameServerSession != nullptr)
	{
		GameServerSession->Disconnect();
		GameServerSession.Reset();
	}

	DestroySocket();
	SetConnectionState(
		ES1ConnectionState::Disconnected,
		Reason);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Game server disconnected. Reason[%s]"),
		*Reason);
}

bool US1NetworkSubsystem::IsConnected() const
{
	return ConnectionState == ES1ConnectionState::Connected &&
		Socket != nullptr &&
		GameServerSession != nullptr;
}

void US1NetworkSubsystem::SendPacket(SendBufferRef SendBuffer)
{
	if (!IsConnected() || SendBuffer == nullptr)
		return;

	GameServerSession->SendPacket(SendBuffer);
}

void US1NetworkSubsystem::HandleRecvPackets()
{
	if (!IsConnected())
		return;

	GameServerSession->HandleRecvPackets();
}

void US1NetworkSubsystem::NotifyLoginPacket(
	const Protocol::S_LOGIN& Packet)
{
	OnLoginPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifySignUpPacket(
	const Protocol::S_SIGN_UP& Packet)
{
	OnSignUpPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyCharacterCreatedPacket(
	const Protocol::S_CREATE_CHARACTER& Packet)
{
	OnCharacterCreatedPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyEnterGamePacket(
	const Protocol::S_ENTER_GAME& Packet)
{
	OnEnterGamePacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyLeaveGamePacket(
	const Protocol::S_LEAVE_GAME& Packet)
{
	OnLeaveGamePacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifySpawnPacket(
	const Protocol::S_SPAWN& Packet)
{
	OnSpawnPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyDespawnPacket(
	const Protocol::S_DESPAWN& Packet)
{
	OnDespawnPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyMovePacket(
	const Protocol::S_MOVE& Packet)
{
	OnMovePacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyChatPacket(
	const Protocol::S_CHAT& Packet)
{
	OnChatPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyCombatActionPacket(
	const Protocol::S_COMBAT_ACTION& Packet)
{
	OnCombatActionPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyCombatActionResultPacket(
	const Protocol::S_COMBAT_ACTION_RESULT& Packet)
{
	OnCombatActionResultPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyMonsterUpdatePacket(const Protocol::S_MONSTER_UPDATE& Packet)
{
	OnMonsterUpdatePacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyMonsterAttackPacket(const Protocol::S_MONSTER_ATTACK& Packet)
{
	OnMonsterAttackPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyDamageResultPacket(const Protocol::S_DAMAGE_RESULT& Packet)
{
	OnDamageResultPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyPlayerStatusPacket(const Protocol::S_PLAYER_STATUS& Packet)
{
	OnPlayerStatusPacket.Broadcast(Packet);
}

bool US1NetworkSubsystem::TickNetwork(float DeltaTime)
{
	HandleRecvPackets();
	return true;
}

void US1NetworkSubsystem::NotifyLootSnapshotPacket(const Protocol::S_LOOT_SNAPSHOT& Packet)
{
 check(IsInGameThread());
 OnLootSnapshotPacket.Broadcast(Packet);
}
void US1NetworkSubsystem::NotifyLootCreatedPacket(const Protocol::S_LOOT_CREATED& Packet)
{
 check(IsInGameThread());
 OnLootCreatedPacket.Broadcast(Packet);
}
void US1NetworkSubsystem::NotifyLootRemovedPacket(const Protocol::S_LOOT_REMOVED& Packet)
{
 check(IsInGameThread());
 OnLootRemovedPacket.Broadcast(Packet);
}
void US1NetworkSubsystem::NotifyItemPickupResultPacket(const Protocol::S_ITEM_PICKUP_RESULT& Packet)
{
 check(IsInGameThread());
 OnItemPickupResultPacket.Broadcast(Packet);
}
void US1NetworkSubsystem::NotifyInventorySnapshotPacket(const Protocol::S_INVENTORY_SNAPSHOT& Packet)
{
 check(IsInGameThread());
 OnInventorySnapshotPacket.Broadcast(Packet);
}

void US1NetworkSubsystem::NotifyItemUseResultPacket(const Protocol::S_ITEM_USE_RESULT& Packet)
{check(IsInGameThread());OnItemUseResultPacket.Broadcast(Packet);}
void US1NetworkSubsystem::NotifyInventoryMoveResultPacket(const Protocol::S_INVENTORY_MOVE_RESULT& Packet)
{check(IsInGameThread());OnInventoryMoveResultPacket.Broadcast(Packet);}
void US1NetworkSubsystem::NotifyPlayerHealedPacket(const Protocol::S_PLAYER_HEALED& Packet)
{check(IsInGameThread());OnPlayerHealedPacket.Broadcast(Packet);}

void US1NetworkSubsystem::DestroySocket()
{
	if (Socket == nullptr)
		return;

	if (ISocketSubsystem* SocketSubsystem =
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
	{
		SocketSubsystem->DestroySocket(Socket);
	}

	Socket = nullptr;
}

void US1NetworkSubsystem::SetConnectionState(
	ES1ConnectionState NewState,
	const FString& Reason)
{
	if (ConnectionState == NewState)
		return;

	ConnectionState = NewState;
	OnConnectionStateChanged.Broadcast(NewState, Reason);
}
