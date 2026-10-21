#include "Game/S1MonsterData.h"
#include "Game/S1DuskMinion.h"
namespace
{
 template<class Row> const Row* Find(const UDataTable* Table,int32 Id)
 {
  if(!Table||Table->GetRowStruct()!=Row::StaticStruct())return nullptr;
  for(const auto& P:Table->GetRowMap()){const auto* R=reinterpret_cast<const Row*>(P.Value);if(R->MonsterDefinitionId==Id)return R;}
  return nullptr;
 }
}
UDataTable* FS1MonsterData::LoadDefinitions(){return LoadObject<UDataTable>(nullptr,TEXT("/Game/S1/Monsters/Data/DT_S1MonsterDefinitions.DT_S1MonsterDefinitions"));}
UDataTable* FS1MonsterData::LoadPresentations(){return LoadObject<UDataTable>(nullptr,TEXT("/Game/S1/Monsters/Data/DT_S1MonsterPresentations.DT_S1MonsterPresentations"));}
const FS1MonsterDefinitionRow* FS1MonsterData::FindDefinition(const UDataTable* T,int32 Id){return Find<FS1MonsterDefinitionRow>(T,Id);}
const FS1MonsterPresentationRow* FS1MonsterData::FindPresentation(const UDataTable* T,int32 Id){return Find<FS1MonsterPresentationRow>(T,Id);}
bool FS1MonsterData::ValidateRow(const FS1MonsterDefinitionRow& D,FString& Error)
{
 for(float V:{D.CapsuleRadius,D.CapsuleHalfHeight,D.MoveSpeed,D.AcquireRadius,D.HomeLeash,D.LoseRadius,D.AttackStartRadius,D.HitRadius,D.HitHeight,D.HitFacingDot})
  if(!FMath::IsFinite(V)){Error=TEXT("Nonfinite monster value");return false;}
 if(D.MonsterDefinitionId<1||D.MonsterDefinitionId>1000000||D.DefinitionVersion<1||D.DefinitionVersion>1000000||D.BehaviorId!=1||
  D.MaxHP<1||D.MaxHP>100000000||D.CapsuleRadius<1||D.CapsuleRadius>100||D.CapsuleHalfHeight<D.CapsuleRadius||D.CapsuleHalfHeight>300||
  D.MoveSpeed<0||D.MoveSpeed>1200||D.AcquireRadius<1||D.AcquireRadius>10000||D.HomeLeash<D.AcquireRadius||D.HomeLeash>20000||
  D.LoseRadius<D.AcquireRadius||D.LoseRadius>20000||D.AttackStartRadius<1||D.HitRadius<D.AttackStartRadius||D.HitRadius>D.AcquireRadius||
  D.HitHeight<1||D.HitHeight>1000||D.HitFacingDot < -1||D.HitFacingDot>1||D.Damage<1||D.Damage>1000000||
  D.AttackDurationMs<100||D.AttackDurationMs>10000||D.AttackIntervalMs<D.AttackDurationMs||D.AttackIntervalMs>60000||
  D.HitDelayMs<1||D.HitDelayMs>=D.AttackDurationMs||D.DespawnMs<100||D.DespawnMs>60000||D.RespawnMs<D.DespawnMs||D.RespawnMs>600000||
  D.HomeRecoveryMs<1||D.HomeRecoveryMs>60000){Error=TEXT("Invalid/unsupported monster definition");return false;}
 Error.Reset();return true;
}
bool FS1MonsterData::Validate(const UDataTable* Definitions,const UDataTable* Presentations,FString& Error)
{
 if(!Definitions||!Presentations||Definitions->GetRowStruct()!=FS1MonsterDefinitionRow::StaticStruct()||Presentations->GetRowStruct()!=FS1MonsterPresentationRow::StaticStruct())
 {Error=TEXT("Missing/wrong monster table types");return false;}
 if(Definitions->GetRowMap().Num()<1||Definitions->GetRowMap().Num()>256||Definitions->GetRowMap().Num()!=Presentations->GetRowMap().Num())
 {Error=TEXT("Monster table counts differ/outside1..256");return false;}
 TSet<int32> Ids;
 for(const auto& Pair:Definitions->GetRowMap())
 {
  const auto& D=*reinterpret_cast<const FS1MonsterDefinitionRow*>(Pair.Value);
  if(!ValidateRow(D,Error)||Ids.Contains(D.MonsterDefinitionId)){Error=TEXT("Invalid/duplicate monster definition");return false;}
  Ids.Add(D.MonsterDefinitionId);
 }
 TSet<int32> VisualIds;
 for(const auto& Pair:Presentations->GetRowMap())
 {
  const auto& P=*reinterpret_cast<const FS1MonsterPresentationRow*>(Pair.Value);
  if(!Ids.Contains(P.MonsterDefinitionId)||VisualIds.Contains(P.MonsterDefinitionId)||P.DisplayName.IsEmpty()||P.DisplayName.ToString().Len()>128||
   !P.ActorClass||P.ActorClass->HasAnyClassFlags(CLASS_Abstract|CLASS_Deprecated|CLASS_NewerVersionExists))
  {Error=TEXT("Missing/invalid/duplicate monster presentation");return false;}
  VisualIds.Add(P.MonsterDefinitionId);
 }
 Error.Reset();return true;
}
bool FS1MonsterData::ExportText(const UDataTable* D,const UDataTable* P,FString& Text,FString& Error)
{
 if(!Validate(D,P,Error))return false;
 TArray<int32> Ids;for(const auto& Pair:D->GetRowMap())Ids.Add(reinterpret_cast<const FS1MonsterDefinitionRow*>(Pair.Value)->MonsterDefinitionId);
 Ids.Sort();FString Out=FString::Printf(TEXT("S1MONSTER 1\nROWS %d\n"),Ids.Num());
 for(int32 Id:Ids)
 {
  const auto& R=*FindDefinition(D,Id);
  Out+=FString::Printf(TEXT("M %d %d %d %d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %d %d %d %d %d %d %d\n"),
   R.MonsterDefinitionId,R.DefinitionVersion,R.BehaviorId,R.MaxHP,R.CapsuleRadius,R.CapsuleHalfHeight,R.MoveSpeed,R.AcquireRadius,R.HomeLeash,
   R.LoseRadius,R.AttackStartRadius,R.HitRadius,R.HitHeight,R.HitFacingDot,R.Damage,R.AttackIntervalMs,R.AttackDurationMs,R.HitDelayMs,R.DespawnMs,R.RespawnMs,R.HomeRecoveryMs);
 }
 Text=MoveTemp(Out);return true;
}
