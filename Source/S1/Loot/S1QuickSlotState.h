#pragma once
#include "CoreMinimal.h"
#include "Loot/S1InventoryState.h"
#include "Misc/SecureHash.h"

struct FS1QuickSlotState
{
 static constexpr int32 SlotCount=4;
 static constexpr int32 Potion=20001;
 TArray<int32> Definitions={0,0,0,0};
 void Reset(){Definitions={0,0,0,0};}
 bool Set(int32 Slot,int32 Definition)
 {
  if(!Definitions.IsValidIndex(Slot)||(Definition!=0&&Definition!=Potion))return false;
  Definitions[Slot]=Definition;return true;
 }
 bool Restore(int32 Version,const FString& SavedHash,const FString& ExpectedHash,const TArray<int32>& Saved)
 {
  Reset();
  if(Version!=1||ExpectedHash.IsEmpty()||SavedHash!=ExpectedHash||Saved.Num()!=SlotCount)return false;
  for(int32 I=0;I<SlotCount;++I)Definitions[I]=Saved[I]==Potion?Potion:0;
  return true;
 }
 uint64 Resolve(int32 Slot,const FS1InventoryState& Inventory) const
 {
  if(!Definitions.IsValidIndex(Slot)||Definitions[Slot]!=Potion||!Inventory.ReadyForUse())return 0;
  uint64 Best=0;uint32 BestSlot=MAX_uint32;
  for(const auto& Entry:Inventory.GetEntries())
   if(Entry.item_definition_id()==Potion&&Entry.count()>0&&
    (Entry.slot_index()<BestSlot||(Entry.slot_index()==BestSlot&&Entry.item_instance_id()<Best)))
   {Best=Entry.item_instance_id();BestSlot=Entry.slot_index();}
  return Best;
 }
 static FString Namespace(const FString& Endpoint,uint64 Account,uint64 Character)
 {
  if(Endpoint.IsEmpty()||!Account||!Character)return {};
  return FMD5::HashAnsiString(*FString::Printf(TEXT("v1|%s|%llu|%llu"),*Endpoint.ToLower(),Account,Character));
 }
};
