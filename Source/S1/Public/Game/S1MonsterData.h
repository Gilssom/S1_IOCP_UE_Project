#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "S1MonsterData.generated.h"
class AS1DuskMinion;
class UTexture2D;

/** Authoring definition only. The deployed IOCP export, never this client's table, decides combat. */
USTRUCT(BlueprintType)
struct S1_API FS1MonsterDefinitionRow : public FTableRowBase
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 MonsterDefinitionId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DefinitionVersion=1;
 // Only 1 (existing Dusk melee FSM) is implemented. A new behavior needs code, not just a row.
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 BehaviorId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 MaxHP=200;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float CapsuleRadius=28;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float CapsuleHalfHeight=62;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float MoveSpeed=220;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float AcquireRadius=800;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HomeLeash=1200;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float LoseRadius=1000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float AttackStartRadius=170;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HitRadius=190;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HitHeight=150;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float HitFacingDot=.57f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Damage=10;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 AttackIntervalMs=2000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 AttackDurationMs=1000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 HitDelayMs=420;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 DespawnMs=3000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 RespawnMs=8000;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 HomeRecoveryMs=4000;
};
USTRUCT(BlueprintType)
struct S1_API FS1MonsterPresentationRow : public FTableRowBase
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 MonsterDefinitionId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FText DisplayName;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TSubclassOf<AS1DuskMinion> ActorClass;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Icon;
 // Mesh/AnimBP/animations/facing/scale remain in ActorClass BP, not duplicated here.
};
struct S1_API FS1MonsterData
{
 static UDataTable* LoadDefinitions();
 static UDataTable* LoadPresentations();
 static const FS1MonsterDefinitionRow* FindDefinition(const UDataTable*,int32 Id);
 static const FS1MonsterPresentationRow* FindPresentation(const UDataTable*,int32 Id);
 static bool ValidateRow(const FS1MonsterDefinitionRow&,FString& Error);
 static bool Validate(const UDataTable*,const UDataTable*,FString& Error);
 static bool ExportText(const UDataTable*,const UDataTable*,FString& Text,FString& Error);
};
