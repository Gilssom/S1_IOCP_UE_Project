// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/PacketSession.h"
#include "NetworkWorker.h"
#include "Sockets.h"
#include "Common/TcpSocketBuilder.h"
#include "Serialization/ArrayWriter.h"
#include "SocketSubsystem.h"
#include "ClientPacketHandler.h"
#include "Network/S1NetworkSubsystem.h"

PacketSession::PacketSession(
	class FSocket* Socket,
	US1NetworkSubsystem* OwnerNetworkSubsystem)
	: Socket(Socket),
	  OwnerNetworkSubsystem(OwnerNetworkSubsystem)
{
	ClientPacketHandler::Init();
}

PacketSession::~PacketSession()
{
	Disconnect();
}

void PacketSession::Run()
{
	RecvWorkerThread = MakeShared<RecvWorker>(Socket, AsShared());
	SendWorkerThread = MakeShared<SendWorker>(Socket, AsShared());
}

void PacketSession::HandleRecvPackets()
{
	while (true)
	{
		TArray<uint8> Packet;
		if (RecvPacketQueue.Dequeue(OUT Packet) == false)
			break;

		// TODO
		PacketSessionRef ThisPtr = AsShared();
		ClientPacketHandler::HandlePacket(ThisPtr, Packet.GetData(), Packet.Num());
	}
}

void PacketSession::SendPacket(SendBufferRef SendBuffer)
{
	if (SendBuffer == nullptr || SendWorkerThread == nullptr)
		return;

	SendPacketQueue.Enqueue(SendBuffer);
	SendWorkerThread->NotifySend();
}

US1NetworkSubsystem* PacketSession::GetOwnerNetworkSubsystem() const
{
	return OwnerNetworkSubsystem.Get();
}

void PacketSession::Disconnect()
{
	if (Socket)
		Socket->Shutdown(ESocketShutdownMode::ReadWrite);

	if (RecvWorkerThread)
	{
		RecvWorkerThread->Destroy();
		RecvWorkerThread = nullptr;
	}

	if (SendWorkerThread)
	{
		SendWorkerThread->Destroy();
		SendWorkerThread = nullptr;
	}

	TArray<uint8> PendingRecvPacket;
	while (RecvPacketQueue.Dequeue(PendingRecvPacket))
	{
	}

	SendBufferRef PendingSendPacket;
	while (SendPacketQueue.Dequeue(PendingSendPacket))
	{
	}

	Socket = nullptr;
	OwnerNetworkSubsystem.Reset();
}
