#pragma once
#include "CoreMinimal.h"
#include "Protocol.pb.h"

// Validated server inventory only; never optimistic increments or local potion use.
class S1_API FS1InventoryState
{
public:
 void Reset(uint64 InCharacter=0){Character=InCharacter;Version=0;MinimumVersion=0;Count=0;bHasSnapshot=false;Entries.Reset();}
 void RequireVersion(uint64 Value){MinimumVersion=FMath::Max(MinimumVersion,Value);}
 bool Apply(const Protocol::S_INVENTORY_SNAPSHOT& Packet)
 {
  if(!Character||Packet.character_id()!=Character||!Packet.success()||
   Packet.reason()!=Protocol::ITEM_REASON_NONE||Packet.entries_size()>20||
   Packet.inventory_version()<MinimumVersion||(bHasSnapshot&&Packet.inventory_version()<Version))return false;
  TSet<uint64> Items;TSet<uint32> Slots;int64 Total=0;
  for(const auto& Entry:Packet.entries())
  {
   if(!Entry.item_instance_id()||!Entry.item_definition_id()||!Entry.count()||Entry.count()>99||
    Entry.slot_index()>=20||Items.Contains(Entry.item_instance_id())||Slots.Contains(Entry.slot_index()))return false;
   Items.Add(Entry.item_instance_id());Slots.Add(Entry.slot_index());
   if(Entry.item_definition_id()==20001)Total+=Entry.count();
  }
  TArray<Protocol::InventoryEntry> NewEntries;
  for(const auto& Entry:Packet.entries())NewEntries.Add(Entry);
  NewEntries.Sort([](const auto& A,const auto& B){return A.slot_index()<B.slot_index();});
  if(bHasSnapshot&&Packet.inventory_version()==Version)
  {
   if(Total!=Count||NewEntries.Num()!=Entries.Num())return false;
   for(int32 I=0;I<Entries.Num();++I)
    if(NewEntries[I].item_instance_id()!=Entries[I].item_instance_id()||NewEntries[I].item_definition_id()!=Entries[I].item_definition_id()||
     NewEntries[I].count()!=Entries[I].count()||NewEntries[I].slot_index()!=Entries[I].slot_index())return false;
  }
  Entries=MoveTemp(NewEntries);Version=Packet.inventory_version();Count=Total;bHasSnapshot=true;return true;
 }
 int64 GetCount() const {return Count;}
 uint64 GetVersion() const {return Version;}
 bool HasSnapshot() const {return bHasSnapshot;}
 const TArray<Protocol::InventoryEntry>& GetEntries() const{return Entries;}
 bool ReadyForUse() const{return bHasSnapshot&&Version>=MinimumVersion;}
private:
 uint64 Character=0,Version=0,MinimumVersion=0;
 int64 Count=0;
 bool bHasSnapshot=false;
 TArray<Protocol::InventoryEntry> Entries;
};
