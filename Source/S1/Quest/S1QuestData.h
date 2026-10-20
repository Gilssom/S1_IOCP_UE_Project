#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "S1QuestData.generated.h"

UENUM(BlueprintType)
enum class ES1DialogueNodeKind:uint8 {Line,QuestOffer,End};

/** Definition only. Never a character's accepted/progress/reward state. */
USTRUCT(BlueprintType)
struct S1_API FS1QuestDefinitionRow:public FTableRowBase
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 QuestId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DefinitionVersion=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bEnabled=false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Title;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(MultiLine=true)) FText Description;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 AcceptNpcId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 ReturnNpcId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 MonsterDefinitionId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 RequiredKills=3;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 RewardItemId=20001;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 RewardCount=3;
};

USTRUCT(BlueprintType)
struct S1_API FS1DialogueNodeRow:public FTableRowBase
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 NodeId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 NpcId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) ES1DialogueNodeKind Kind=ES1DialogueNodeKind::Line;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText Speaker;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(MultiLine=true)) FText Body;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 NextNodeId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 QuestId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 AcceptNodeId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DeclineNodeId=0;
};

/** Shared validation for preview and offline export. No network or persistence. */
struct S1_API FS1QuestData
{
 static bool Validate(const UDataTable* Quests,const UDataTable* Nodes,FString& Error);
 static const FS1DialogueNodeRow* FindNode(const UDataTable* Nodes,int32 Id);
 static const FS1QuestDefinitionRow* FindQuest(const UDataTable* Quests,int32 Id);
 static bool ExportText(const UDataTable* Quests,const UDataTable* Nodes,FString& Text,FString& Error);
};

struct S1_API FS1DialogueSession
{
 int32 CurrentNodeId=0;
 bool Start(const UDataTable* Quests,const UDataTable* Nodes,int32 NpcId,int32 StartId,FString& Error);
 bool Advance(const UDataTable* Nodes);
 // Explicitly local preview; never an accepted-quest result.
 bool ChoosePreview(const UDataTable* Nodes,bool bAccept,bool bPreviewAllowed);
 void Reset(){CurrentNodeId=0;}
};
