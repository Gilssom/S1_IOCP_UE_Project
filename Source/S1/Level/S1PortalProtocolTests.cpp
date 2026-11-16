#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ClientPacketHandler.h"
#include "Network/PacketSession.h"
#include "Network/S1NetworkSubsystem.h"
#include "Engine/GameInstance.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PortalProtocolTest,"S1.Level.PortalProtocol",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1PortalProtocolTest::RunTest(const FString&)
{
 TestEqual(TEXT("Existing quest packet ID unchanged"),int32(PKT_S_QUEST_TURN_IN_RESULT),1046);
 TestEqual(TEXT("Portal request ID"),int32(PKT_C_PORTAL_TRANSFER_REQUEST),1047);
 TestEqual(TEXT("Portal commit ID"),int32(PKT_S_PORTAL_TRANSFER_COMMIT),1050);
 const std::string Id("\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17",16);
 Protocol::C_PORTAL_TRANSFER_REQUEST Request;Request.set_request_id(Id);Request.set_portal_id(1);Request.set_contract_version(1);
 auto Buffer=ClientPacketHandler::MakeSendBuffer(Request);
 TestNotNull(TEXT("Portal request framed"),Buffer.Get());if(!Buffer)return false;
 const auto* Header=reinterpret_cast<const PacketHeader*>(Buffer->Buffer());
 TestEqual(TEXT("Request header ID"),int32(Header->id),1047);
 Protocol::C_PORTAL_TRANSFER_REQUEST Parsed;
 TestTrue(TEXT("Request payload parses"),Parsed.ParseFromArray(Buffer->Buffer()+sizeof(PacketHeader),Buffer->WriteSize()-sizeof(PacketHeader)));
 TestTrue(TEXT("Binary request ID preserved"),Parsed.request_id()==Id);
 auto* Instance=NewObject<UGameInstance>();auto* Network=NewObject<US1NetworkSubsystem>(Instance);
 PacketSessionRef Session=MakeShared<PacketSession>(nullptr,Network);int32 Received=0;
 Network->OnPortalPreparePacket.AddLambda([&](const Protocol::S_PORTAL_TRANSFER_PREPARE& P)
 {++Received;TestTrue(TEXT("Prepare request ID"),P.request_id()==Id);TestEqual(TEXT("Prepare generation"),P.generation(),uint64(99));});
 Network->OnPortalCommitPacket.AddLambda([&](const Protocol::S_PORTAL_TRANSFER_COMMIT& P)
 {++Received;TestEqual(TEXT("Destination region"),P.destination_region(),1u);});
 auto Dispatch=[&](const auto& Packet,uint16 PacketId)
 {
  std::string Payload;Packet.SerializeToString(&Payload);TArray<uint8> Frame;
  Frame.SetNumUninitialized(sizeof(PacketHeader)+Payload.size());auto* H=reinterpret_cast<PacketHeader*>(Frame.GetData());
  H->size=Frame.Num();H->id=PacketId;FMemory::Memcpy(Frame.GetData()+sizeof(PacketHeader),Payload.data(),Payload.size());
  TestTrue(TEXT("Portal dispatch"),ClientPacketHandler::HandlePacket(Session,Frame.GetData(),Frame.Num()));
 };
 Protocol::S_PORTAL_TRANSFER_PREPARE Prepare;Prepare.set_request_id(Id);Prepare.set_generation(99);Prepare.set_accepted(true);
 Protocol::S_PORTAL_TRANSFER_COMMIT Commit;Commit.set_request_id(Id);Commit.set_generation(99);Commit.set_destination_region(1);
 Dispatch(Prepare,PKT_S_PORTAL_TRANSFER_PREPARE);Dispatch(Commit,PKT_S_PORTAL_TRANSFER_COMMIT);
 TestEqual(TEXT("Both events dispatched"),Received,2);Session->Disconnect();return true;
}
#endif
