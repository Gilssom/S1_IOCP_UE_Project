#pragma once
#include "CoreMinimal.h"
#include "Protocol.pb.h"

// Initial-entry contract only. Live region transfers require a separate lifecycle contract.
struct FS1RegionContract
{
 static constexpr uint32 Version=1,Combat=1,Village=2;
 static FName MapFor(uint32 Region)
 {
  if(Region==Combat)return FName(TEXT("/Game/Maps/MainMap"));
  if(Region==Village)return FName(TEXT("/Game/Maps/VillageMap"));
  return NAME_None;
 }
 static bool Supports(const Protocol::S_ENTER_GAME& Packet)
 {
  if(Packet.region_contract_version()!=Version||MapFor(Packet.region_id()).IsNone()||
   !Packet.has_player()||!Packet.player().has_character_info()||!Packet.player().object_id()||
   !Packet.player().character_info().character_id()||!Packet.player().has_pos_info())return false;
  const auto& P=Packet.player().pos_info();
  return FMath::IsFinite(P.x())&&FMath::IsFinite(P.y())&&FMath::IsFinite(P.z())&&FMath::IsFinite(P.yaw());
 }
};
