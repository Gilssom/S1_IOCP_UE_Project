#include "Quest/S1QuestData.h"
#include <type_traits>

namespace
{
 template<class Row> const Row* Find(const UDataTable* Table,int32 Id)
 {
  if(!Table||Table->GetRowStruct()!=Row::StaticStruct())return nullptr;
  for(const auto& Pair:Table->GetRowMap())
  {
   const auto* Value=reinterpret_cast<const Row*>(Pair.Value);
   if constexpr(std::is_same_v<Row,FS1QuestDefinitionRow>){if(Value->QuestId==Id)return Value;}
   else {if(Value->NodeId==Id)return Value;}
  }
  return nullptr;
 }
 FString Quote(FString Value)
 {
  Value.ReplaceInline(TEXT("\\"),TEXT("\\\\"));Value.ReplaceInline(TEXT("\""),TEXT("\\\""));
  return TEXT("\"")+Value+TEXT("\""); // std::quoted-compatible; embedded newlines remain quoted.
 }
}
const FS1DialogueNodeRow* FS1QuestData::FindNode(const UDataTable* T,int32 Id){return Find<FS1DialogueNodeRow>(T,Id);}
const FS1QuestDefinitionRow* FS1QuestData::FindQuest(const UDataTable* T,int32 Id){return Find<FS1QuestDefinitionRow>(T,Id);}
bool FS1QuestData::Validate(const UDataTable* Quests,const UDataTable* Nodes,FString& Error)
{
 auto Fail=[&](const FString& Why){Error=Why;return false;};
 if(!Quests||!Nodes||Quests->GetRowStruct()!=FS1QuestDefinitionRow::StaticStruct()||Nodes->GetRowStruct()!=FS1DialogueNodeRow::StaticStruct())return Fail(TEXT("Missing/wrong table row type"));
 if(Quests->GetRowMap().Num()<1||Quests->GetRowMap().Num()>256||Nodes->GetRowMap().Num()<1||Nodes->GetRowMap().Num()>4096)return Fail(TEXT("Table count outside supported limits"));
 TSet<int32> QuestIds,NodeIds,NpcIds;
 TMap<int32,int32> InDegree;
 TMap<int32,TArray<int32>> Edges;
 for(const auto& Pair:Nodes->GetRowMap())
 {
  const auto& D=*reinterpret_cast<const FS1DialogueNodeRow*>(Pair.Value);
  if(D.NodeId<=0||D.NpcId<=0||NodeIds.Contains(D.NodeId)||D.Body.IsEmpty()||D.Speaker.IsEmpty()||D.Body.ToString().Len()>8192||D.Speaker.ToString().Len()>256)return Fail(FString::Printf(TEXT("Invalid/duplicate dialogue: %s"),*Pair.Key.ToString()));
  NodeIds.Add(D.NodeId);NpcIds.Add(D.NpcId);InDegree.Add(D.NodeId,0);
 }
 for(const auto& Pair:Quests->GetRowMap())
 {
  const auto& Q=*reinterpret_cast<const FS1QuestDefinitionRow*>(Pair.Value);
  // Current registered server definitions: Dusk=1, recovery potion=20001. Server revalidates.
  if(Q.QuestId<=0||QuestIds.Contains(Q.QuestId)||Q.DefinitionVersion<=0||Q.Title.IsEmpty()||Q.Description.IsEmpty()||
   Q.Title.ToString().Len()>256||Q.Description.ToString().Len()>8192||!NpcIds.Contains(Q.AcceptNpcId)||!NpcIds.Contains(Q.ReturnNpcId)||
   Q.MonsterDefinitionId!=1||Q.RewardItemId!=20001||Q.RequiredKills<1||Q.RequiredKills>100000||Q.RewardCount<1||Q.RewardCount>99)
   return Fail(FString::Printf(TEXT("Invalid/duplicate/unregistered quest: %s"),*Pair.Key.ToString()));
  QuestIds.Add(Q.QuestId);
 }
 for(const auto& Pair:Nodes->GetRowMap())
 {
  const auto& D=*reinterpret_cast<const FS1DialogueNodeRow*>(Pair.Value);
  TArray<int32> Links;
  if(D.Kind==ES1DialogueNodeKind::Line)
  {if(D.NextNodeId<=0||D.QuestId||D.AcceptNodeId||D.DeclineNodeId)return Fail(TEXT("Line needs only NextNodeId"));Links.Add(D.NextNodeId);}
  else if(D.Kind==ES1DialogueNodeKind::QuestOffer)
  {
   const auto* Q=FindQuest(Quests,D.QuestId);
   if(!Q||Q->AcceptNpcId!=D.NpcId||D.NextNodeId||D.AcceptNodeId<=0||D.DeclineNodeId<=0)return Fail(TEXT("QuestOffer needs matching quest/NPC and both branches"));
   Links={D.AcceptNodeId,D.DeclineNodeId};
  }
  else if(D.Kind!=ES1DialogueNodeKind::End||D.NextNodeId||D.QuestId||D.AcceptNodeId||D.DeclineNodeId)return Fail(TEXT("End must not link to other nodes"));
  for(int32 Link:Links)
  {
   const auto* Target=FindNode(Nodes,Link);
   if(!Target||Target->NpcId!=D.NpcId)return Fail(TEXT("Missing/cross-NPC dialogue link"));
   ++InDegree.FindChecked(Link);
  }
  Edges.Add(D.NodeId,Links);
 }
 TArray<int32> Queue;for(const auto& Pair:InDegree)if(Pair.Value==0)Queue.Add(Pair.Key);
 for(int32 I=0;I<Queue.Num();++I)for(int32 Link:Edges.FindChecked(Queue[I]))if(--InDegree.FindChecked(Link)==0)Queue.Add(Link);
 if(Queue.Num()!=NodeIds.Num())return Fail(TEXT("Dialogue cycle detected (including disconnected rows)"));
 Error.Reset();return true;
}
bool FS1QuestData::ExportText(const UDataTable* Quests,const UDataTable* Nodes,FString& Text,FString& Error)
{
 Text.Reset();if(!Validate(Quests,Nodes,Error))return false;
 FString Out=FString::Printf(TEXT("S1QUEST 1\nQUESTS %d\n"),Quests->GetRowMap().Num());
 TArray<int32> Ids;for(const auto& Pair:Quests->GetRowMap())Ids.Add(reinterpret_cast<const FS1QuestDefinitionRow*>(Pair.Value)->QuestId);Ids.Sort();
 for(int32 Id:Ids)
 {
  const auto& Q=*FindQuest(Quests,Id);
  Out+=FString::Printf(TEXT("Q %d %d %d %d %d %d %d %d %d "),Q.QuestId,Q.DefinitionVersion,Q.bEnabled?1:0,Q.AcceptNpcId,Q.ReturnNpcId,Q.MonsterDefinitionId,Q.RequiredKills,Q.RewardItemId,Q.RewardCount)+Quote(Q.Title.ToString())+TEXT(" ")+Quote(Q.Description.ToString())+TEXT("\n");
 }
 Out+=FString::Printf(TEXT("DIALOGUES %d\n"),Nodes->GetRowMap().Num());Ids.Reset();
 for(const auto& Pair:Nodes->GetRowMap())Ids.Add(reinterpret_cast<const FS1DialogueNodeRow*>(Pair.Value)->NodeId);Ids.Sort();
 for(int32 Id:Ids)
 {
  const auto& D=*FindNode(Nodes,Id);
  Out+=FString::Printf(TEXT("D %d %d %d %d %d %d %d "),D.NodeId,D.NpcId,static_cast<int32>(D.Kind),D.NextNodeId,D.QuestId,D.AcceptNodeId,D.DeclineNodeId)+Quote(D.Speaker.ToString())+TEXT(" ")+Quote(D.Body.ToString())+TEXT("\n");
 }
 if(FTCHARToUTF8(*Out).Length()>1048576){Error=TEXT("Export exceeds 1 MiB");return false;}
 Text=MoveTemp(Out);return true;
}
bool FS1DialogueSession::Start(const UDataTable* Q,const UDataTable* D,int32 NpcId,int32 StartId,FString& Error)
{
 Reset();if(!FS1QuestData::Validate(Q,D,Error))return false;
 const auto* Node=FS1QuestData::FindNode(D,StartId);
 if(!Node||Node->NpcId!=NpcId){Error=TEXT("Missing/wrong-NPC start node");return false;}
 CurrentNodeId=StartId;return true;
}
bool FS1DialogueSession::Advance(const UDataTable* D)
{
 const auto* Node=FS1QuestData::FindNode(D,CurrentNodeId);
 if(!Node||Node->Kind!=ES1DialogueNodeKind::Line||!FS1QuestData::FindNode(D,Node->NextNodeId))return false;
 CurrentNodeId=Node->NextNodeId;return true;
}
bool FS1DialogueSession::ChoosePreview(const UDataTable* D,bool bAccept,bool bPreviewAllowed)
{
 const auto* Node=FS1QuestData::FindNode(D,CurrentNodeId);
 if(!bPreviewAllowed||!Node||Node->Kind!=ES1DialogueNodeKind::QuestOffer)return false;
 const int32 Next=bAccept?Node->AcceptNodeId:Node->DeclineNodeId;
 if(!FS1QuestData::FindNode(D,Next))return false;
 CurrentNodeId=Next;return true;
}
