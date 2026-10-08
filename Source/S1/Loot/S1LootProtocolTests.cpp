#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "ClientPacketHandler.h"
#include "Network/PacketSession.h"
#include "Network/S1NetworkSubsystem.h"
#include "Engine/GameInstance.h"
#include "Protocol.pb.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1LootProtocolTest, "S1.Loot.ProtocolTransport",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FS1LootProtocolTest::RunTest(const FString& Parameters)
{
 TestEqual(TEXT("Existing combat packet ID preserved"), int32(PKT_S_COMBAT_ACTION_RESULT), 1022);
 TestEqual(TEXT("Snapshot ID"), int32(PKT_S_LOOT_SNAPSHOT), 1023);
 TestEqual(TEXT("Created ID"), int32(PKT_S_LOOT_CREATED), 1024);
 TestEqual(TEXT("Removed ID"), int32(PKT_S_LOOT_REMOVED), 1025);
 TestEqual(TEXT("Pickup request ID"), int32(PKT_C_ITEM_PICKUP), 1026);
 TestEqual(TEXT("Pickup result ID"), int32(PKT_S_ITEM_PICKUP_RESULT), 1027);
 TestEqual(TEXT("Inventory query ID"), int32(PKT_C_INVENTORY_QUERY), 1028);
 TestEqual(TEXT("Inventory snapshot ID"), int32(PKT_S_INVENTORY_SNAPSHOT), 1029);

 Protocol::C_ENTER_GAME LegacyRequest;
 LegacyRequest.set_character_id(123);
 std::string Wire;
 LegacyRequest.SerializeToString(&Wire);
 Protocol::C_ENTER_GAME ParsedRequest;
 TestTrue(TEXT("Legacy entry request parses"), ParsedRequest.ParseFromString(Wire));
 TestEqual(TEXT("Absent request version does not enable loot"), ParsedRequest.loot_contract_version(), 0u);
 TestEqual(TEXT("Legacy request does not opt into presentation"),ParsedRequest.loot_presentation_version(),0u);
 Protocol::S_ENTER_GAME LegacyResponse;
 LegacyResponse.set_combat_contract_version(2);
 LegacyResponse.SerializeToString(&Wire);
 Protocol::S_ENTER_GAME ParsedResponse;
 TestTrue(TEXT("Legacy entry response parses"), ParsedResponse.ParseFromString(Wire));
 TestEqual(TEXT("Absent server version does not enable loot"), ParsedResponse.loot_contract_version(), 0u);
 TestEqual(TEXT("Combat version unchanged"), ParsedResponse.combat_contract_version(), 2u);
 TestEqual(TEXT("Legacy response does not enable presentation"),ParsedResponse.loot_presentation_version(),0u);
 LegacyRequest.set_loot_presentation_version(1);
 LegacyRequest.SerializeToString(&Wire);ParsedRequest.ParseFromString(Wire);
 TestEqual(TEXT("Presentation request survives roundtrip"),ParsedRequest.loot_presentation_version(),1u);
 TestEqual(TEXT("Presentation does not opt into grants"),ParsedRequest.loot_contract_version(),0u);
 LegacyResponse.set_loot_presentation_version(1);
 LegacyResponse.SerializeToString(&Wire);ParsedResponse.ParseFromString(Wire);
 TestEqual(TEXT("Presentation accepted separately"),ParsedResponse.loot_presentation_version(),1u);
 TestEqual(TEXT("Full loot remains disabled"),ParsedResponse.loot_contract_version(),0u);

 // Binary IDs must survive embedded zeroes, not UTF8 conversion or textual encoding.
 const std::string DropId("\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17", 16);
 const std::string RequestId(16, '\x7f');
 Protocol::C_ITEM_PICKUP Request;
 Request.set_drop_id(DropId);
 Request.set_request_id(RequestId);
 Request.set_contract_version(1);
 auto Buffer = ClientPacketHandler::MakeSendBuffer(Request);
 TestNotNull(TEXT("Pickup framed buffer"), Buffer.Get());
 if (!Buffer) return false;
 const auto* Header = reinterpret_cast<const PacketHeader*>(Buffer->Buffer());
 TestEqual(TEXT("Pickup framed packet id"), int32(Header->id), 1026);
 TestEqual(TEXT("Pickup frame size"), int32(Header->size), Buffer->WriteSize());
 Protocol::C_ITEM_PICKUP ParsedPickup;
 TestTrue(TEXT("Pickup payload parses"), ParsedPickup.ParseFromArray(Buffer->Buffer() + sizeof(PacketHeader),
  Buffer->WriteSize() - sizeof(PacketHeader)));
 TestTrue(TEXT("Drop ID byte-exact"), ParsedPickup.drop_id() == DropId);
 TestTrue(TEXT("Request ID byte-exact"), ParsedPickup.request_id() == RequestId);
 const unsigned char Truncated[] = {0x0a, 0x10, 0x01};
 TestFalse(TEXT("Truncated pickup protobuf rejected"), ParsedPickup.ParseFromArray(Truncated, sizeof(Truncated)));

 // Dispatch through the actual generated handler and game-thread network events.
 auto* GameInstance = NewObject<UGameInstance>();
 auto* Network = NewObject<US1NetworkSubsystem>(GameInstance);
 PacketSessionRef Session = MakeShared<PacketSession>(nullptr, Network);
 int32 Received = 0;
 Network->OnLootSnapshotPacket.AddLambda([&](const Protocol::S_LOOT_SNAPSHOT& Packet)
 { ++Received; TestEqual(TEXT("Snapshot version routed"), Packet.contract_version(), 1u); });
 Network->OnLootCreatedPacket.AddLambda([&](const Protocol::S_LOOT_CREATED& Packet)
 { ++Received; TestTrue(TEXT("Created ID routed"), Packet.drop().drop_id() == DropId); });
 Network->OnLootRemovedPacket.AddLambda([&](const Protocol::S_LOOT_REMOVED& Packet)
 { ++Received; TestEqual(TEXT("Expiry routed"), int32(Packet.reason()), int32(Protocol::LOOT_REMOVAL_EXPIRED)); });
 Network->OnItemPickupResultPacket.AddLambda([&](const Protocol::S_ITEM_PICKUP_RESULT& Packet)
 { ++Received; TestEqual(TEXT("Unconfirmed stays distinct"), int32(Packet.decision()), int32(Protocol::ITEM_PICKUP_UNCONFIRMED)); });
 Network->OnInventorySnapshotPacket.AddLambda([&](const Protocol::S_INVENTORY_SNAPSHOT& Packet)
 { ++Received; TestEqual(TEXT("64-bit version routed"), Packet.inventory_version(), uint64(1) << 40); });
 auto Dispatch = [&](const auto& Packet, uint16 Id)
 {
  std::string Payload;
  Packet.SerializeToString(&Payload);
  TArray<uint8> Frame;
  Frame.SetNumUninitialized(sizeof(PacketHeader) + Payload.size());
  auto* FrameHeader = reinterpret_cast<PacketHeader*>(Frame.GetData());
  FrameHeader->size = Frame.Num(); FrameHeader->id = Id;
  FMemory::Memcpy(Frame.GetData() + sizeof(PacketHeader), Payload.data(), Payload.size());
  TestTrue(TEXT("Generated dispatch accepts typed loot packet"),
   ClientPacketHandler::HandlePacket(Session, Frame.GetData(), Frame.Num()));
 };
 Protocol::S_LOOT_SNAPSHOT Snapshot; Snapshot.set_contract_version(1);
 Dispatch(Snapshot, PKT_S_LOOT_SNAPSHOT);
 Protocol::S_LOOT_CREATED Created; Created.mutable_drop()->set_drop_id(DropId);
 Dispatch(Created, PKT_S_LOOT_CREATED);
 Protocol::S_LOOT_REMOVED Removed; Removed.set_reason(Protocol::LOOT_REMOVAL_EXPIRED);
 Dispatch(Removed, PKT_S_LOOT_REMOVED);
 Protocol::S_ITEM_PICKUP_RESULT Result; Result.set_decision(Protocol::ITEM_PICKUP_UNCONFIRMED);
 Dispatch(Result, PKT_S_ITEM_PICKUP_RESULT);
 Protocol::S_INVENTORY_SNAPSHOT Inventory; Inventory.set_inventory_version(uint64(1) << 40);
 Dispatch(Inventory, PKT_S_INVENTORY_SNAPSHOT);
 TestEqual(TEXT("Exactly five typed events, no world or inventory mutation"), Received, 5);
 Session->Disconnect();
 return true;
}
#endif
