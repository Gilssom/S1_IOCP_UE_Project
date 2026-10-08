#include "Loot/S1LootState.h"
#include "Protocol.pb.h"

bool FS1LootState::KeyFromBytes(const std::string& Bytes,FString& Key)
{
 if(Bytes.size()!=16)return false;
 bool bAny=false;Key.Reset();
 for(unsigned char Byte:Bytes){bAny|=Byte!=0;Key+=FString::Printf(TEXT("%02x"),Byte);}
 return bAny;
}
bool FS1LootState::Decode(const Protocol::LootDropInfo& Packet,FS1LootPresentation& Result)
{
 FS1LootPresentation Value;
 if(!KeyFromBytes(Packet.drop_id(),Value.Key)||!Packet.owner_character_id()||
  Packet.item_definition_id()!=20001||Packet.count()!=1||Packet.definition_version()!=1||
  !Packet.expires_utc_ms()||!FMath::IsFinite(Packet.x())||!FMath::IsFinite(Packet.y())||!FMath::IsFinite(Packet.z()))return false;
 Value.OwnerCharacterId=Packet.owner_character_id();
 Value.Floor=FVector(Packet.x(),Packet.y(),Packet.z());Value.ExpiresUtcMs=Packet.expires_utc_ms();
 Result=Value;return true;
}
void FS1LootState::Reset(bool bInEnabled)
{bEnabled=bInEnabled;bHasSnapshot=false;Sequence=0;Drops.Reset();}
bool FS1LootState::ApplySnapshot(const Protocol::S_LOOT_SNAPSHOT& Packet)
{
 if(!bEnabled||Packet.contract_version()!=1||Packet.drops_size()>1024||
  (bHasSnapshot&&Packet.snapshot_sequence()<Sequence))return false;
 if(bHasSnapshot&&Packet.snapshot_sequence()==Sequence)return true;
 TMap<FString,FS1LootPresentation> Next;
 for(const auto& Drop:Packet.drops())
 {
  FS1LootPresentation Value;if(!Decode(Drop,Value)||Next.Contains(Value.Key))return false;
  Next.Add(Value.Key,Value);
 }
 Drops=MoveTemp(Next);Sequence=Packet.snapshot_sequence();bHasSnapshot=true;return true;
}
bool FS1LootState::ApplyCreated(const Protocol::S_LOOT_CREATED& Packet)
{
 if(!bEnabled||!bHasSnapshot||!Packet.event_sequence()||Packet.event_sequence()<=Sequence)return false;
 FS1LootPresentation Value;if(!Decode(Packet.drop(),Value))return false;
 if(const auto* Existing=Drops.Find(Value.Key))
 {
  if(Existing->OwnerCharacterId!=Value.OwnerCharacterId||Existing->Floor!=Value.Floor||
   Existing->ExpiresUtcMs!=Value.ExpiresUtcMs)return false;
 }
 if(Drops.Num()>=1024&&!Drops.Contains(Value.Key))return false;
 Drops.Add(Value.Key,Value);Sequence=Packet.event_sequence();return true;
}
bool FS1LootState::ApplyRemoved(const Protocol::S_LOOT_REMOVED& Packet)
{
 if(!bEnabled||!bHasSnapshot||!Packet.event_sequence()||Packet.event_sequence()<=Sequence||
  (Packet.reason()!=Protocol::LOOT_REMOVAL_PICKED_UP&&Packet.reason()!=Protocol::LOOT_REMOVAL_EXPIRED))return false;
 FString Key;if(!KeyFromBytes(Packet.drop_id(),Key))return false;
 Drops.Remove(Key);Sequence=Packet.event_sequence();return true;
}
TArray<FS1LootPresentation> FS1LootState::GetDrops() const
{TArray<FS1LootPresentation> Result;Drops.GenerateValueArray(Result);return Result;}
