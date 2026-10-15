#pragma once
#include "CoreMinimal.h"
#include <string>
namespace Protocol {class LootDropInfo;class S_LOOT_SNAPSHOT;class S_LOOT_CREATED;class S_LOOT_REMOVED;}

struct FS1LootPresentation
{
 FString Key;
 uint64 OwnerCharacterId=0;
 uint32 SourceKind=0,Count=1;
 FVector Floor=FVector::ZeroVector;
 uint64 ExpiresUtcMs=0;
};

// Level-flow data only. Does not spawn actors, mutate inventory, or expire locally.
class S1_API FS1LootState
{
public:
 void Reset(bool bEnabled=false);
 bool ApplySnapshot(const Protocol::S_LOOT_SNAPSHOT& Packet);
 bool ApplyCreated(const Protocol::S_LOOT_CREATED& Packet);
 bool ApplyRemoved(const Protocol::S_LOOT_REMOVED& Packet);
 TArray<FS1LootPresentation> GetDrops() const;
 bool IsEnabled() const {return bEnabled;}
 uint64 GetSequence() const {return Sequence;}
 static bool Decode(const Protocol::LootDropInfo& Packet,FS1LootPresentation& Result);
 static bool KeyFromBytes(const std::string& Bytes,FString& Key);
private:
 bool bEnabled=false,bHasSnapshot=false;
 uint64 Sequence=0;
 TMap<FString,FS1LootPresentation> Drops;
};
