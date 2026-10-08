// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetworkWorker.h"
#include "Sockets.h"
#include "Serialization/ArrayWriter.h"
#include "PacketSession.h"
#include "HAL/PlatformProcess.h"

RecvWorker::RecvWorker(FSocket* Socket, TSharedPtr<class PacketSession> Session) : Socket(Socket), SessionRef(Session)
{
	Thread = FRunnableThread::Create(this, TEXT("RecvWorkerThread"));
}

RecvWorker::~RecvWorker()
{
	Destroy();
}

bool RecvWorker::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Recv Thread Init")));
	return true;
}

uint32 RecvWorker::Run()
{
	while (Running)
	{
		if (Socket == nullptr)
			break;

		// 데이터가 없을 때 CPU를 점유하지 않고 소켓 이벤트를 기다린다.
		if (Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(100)) == false)
			continue;

		TArray<uint8> Packet;

		if (ReceivePacket(OUT Packet))
		{
			if (TSharedPtr<PacketSession> Session = SessionRef.Pin())
			{
				Session->RecvPacketQueue.Enqueue(MoveTemp(Packet));
			}
		}
	}

	return 0;
}

void RecvWorker::Exit()
{

}

void RecvWorker::Destroy()
{
	if (Thread == nullptr)
		return;

	Running = false;
	Thread->WaitForCompletion();
	delete Thread;
	Thread = nullptr;
}

bool RecvWorker::ReceivePacket(TArray<uint8>& OutPacket)
{
	// 패킷 헤더 파싱
	const int32 HeaderSize = sizeof(FPacketHeader);
	TArray<uint8> HeaderBuffer;
	HeaderBuffer.AddZeroed(HeaderSize);

	if (ReceiveDesiredBytes(HeaderBuffer.GetData(), HeaderSize) == false)
		return false;

	// ID, Size 추출
	FPacketHeader Header;
	{
		FMemoryReader Reader(HeaderBuffer);
		Reader << Header;
		UE_LOG(LogTemp, VeryVerbose, TEXT("Recv PacketID : %d, PacketSize : %d"), Header.PacketID, Header.PacketSize);
	}

	if (Header.PacketSize < HeaderSize)
		return false;

	// 패킷 헤더 복사
	OutPacket = MoveTemp(HeaderBuffer);

	// 패킷 내용 파싱
	const int32 PayloadSize = Header.PacketSize - HeaderSize;
	if (PayloadSize == 0)
		return true;

	OutPacket.AddZeroed(PayloadSize);

	if (ReceiveDesiredBytes(&OutPacket[HeaderSize], PayloadSize))
		return true;

	return false;
}

bool RecvWorker::ReceiveDesiredBytes(uint8* Results, int32 Size)
{
	if (Socket == nullptr || Results == nullptr || Size <= 0)
		return false;

	int32 Offset = 0;

	while (Running && Size > 0)
	{
		if (Socket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(100)) == false)
			continue;

		int32 NumRead = 0;
		if (Socket->Recv(Results + Offset, Size, OUT NumRead) == false || NumRead <= 0)
			return false;

		Offset += NumRead;
		Size -= NumRead;
	}

	return Size == 0;
}

// SendWorker
SendWorker::SendWorker(FSocket* Socket, TSharedPtr<PacketSession> Session) : Socket(Socket), SessionRef(Session)
{
	SendEvent = FPlatformProcess::GetSynchEventFromPool(false);
	Thread = FRunnableThread::Create(this, TEXT("SendWorkerThread"));
}

SendWorker::~SendWorker()
{
	Destroy();
	if (SendEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(SendEvent);
		SendEvent = nullptr;
	}
}

bool SendWorker::Init()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Send Thread Init")));

	return true;
}

uint32 SendWorker::Run()
{
	while (Running)
	{
		if (SendEvent)
			SendEvent->Wait();

		if (Running == false)
			break;

		TSharedPtr<PacketSession> Session = SessionRef.Pin();
		if (Session == nullptr)
			break;

		SendBufferRef SendBuffer;
		while (Session->SendPacketQueue.Dequeue(OUT SendBuffer))
		{
			if (SendPacket(SendBuffer) == false)
				break;
		}
	}

	return 0;
}

void SendWorker::Exit()
{

}

bool SendWorker::SendPacket(SendBufferRef SendBuffer)
{
	if (SendBuffer == nullptr)
		return false;

	if (SendDesiredBytes(SendBuffer->Buffer(), SendBuffer->WriteSize()) == false)
		return false;

	return true;
}

void SendWorker::NotifySend()
{
	if (SendEvent)
		SendEvent->Trigger();
}

void SendWorker::Destroy()
{
	if (Thread == nullptr)
		return;

	Running = false;
	NotifySend();
	Thread->WaitForCompletion();
	delete Thread;
	Thread = nullptr;
}

bool SendWorker::SendDesiredBytes(const uint8* Buffer, int32 Size)
{
	while (Running && Size > 0)
	{
		int32 BytesSent = 0;
		if (Socket == nullptr || Socket->Send(Buffer, Size, BytesSent) == false || BytesSent <= 0)
			return false;

		Size -= BytesSent;
		Buffer += BytesSent;
	}

	return Size == 0;
}
