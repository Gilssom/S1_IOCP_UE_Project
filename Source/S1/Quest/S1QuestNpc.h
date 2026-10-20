#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "S1QuestNpc.generated.h"
class USkeletalMeshComponent;
class USkeletalMesh;
class UAnimSequence;
class UWidgetComponent;
class US1DialogueWidget;
class US1NpcNameplateComponent;
class UDataTable;

/** Fixed, local presentation only. Future server NPC definition must match NpcId/placement. */
UCLASS()
class S1_API AS1QuestNpc : public AActor
{
 GENERATED_BODY()
public:
 AS1QuestNpc();
 virtual void OnConstruction(const FTransform& Transform) override;
 virtual void BeginPlay() override;
 virtual void Tick(float DeltaSeconds) override;
 void ShowPrompt(bool bShow);
 /** Call after a runtime DisplayName change; editor changes refresh on construction. */
 UFUNCTION(BlueprintCallable,Category="NPC") void RefreshNameplate();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="NPC") TObjectPtr<US1NpcNameplateComponent> Nameplate;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="NPC") TObjectPtr<USkeletalMeshComponent> Visual;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC") int32 NpcId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC") FText DisplayName;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Appearance") TObjectPtr<USkeletalMesh> NpcMesh;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Appearance") TObjectPtr<UAnimSequence> IdleAnimation;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Appearance") FVector MeshOffset=FVector::ZeroVector;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Appearance") float MeshYaw=-90.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Appearance") FVector MeshScale=FVector::OneVector;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue",meta=(MultiLine=true)) FText Dialogue;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue") FText QuestTitle;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue",meta=(MultiLine=true)) FText QuestPreview;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue") TSubclassOf<US1DialogueWidget> DialogueWidgetClass;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") TObjectPtr<UDataTable> QuestDefinitions;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") TObjectPtr<UDataTable> DialogueNodes;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") int32 StartDialogueNodeId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") int32 DialogueQuestId=1;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") int32 TurnInDialogueNodeId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") int32 RewardedDialogueNodeId=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") int32 InProgressDialogueNodeId=0;
 /** Test branches only; never sends acceptance or saves progress. */
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue Data") bool bPreviewQuestBranches=false;
 UFUNCTION(CallInEditor,Category="NPC|Dialogue Data") void ExportQuestData();
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue",meta=(ClampMin="50")) float InteractionRadius=250.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Dialogue",meta=(ClampMin="50")) float ExitRadius=350.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Camera") FVector CameraOffset=FVector(-320.f,240.f,160.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Camera") float FocusHeight=100.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Camera",meta=(ClampMin="30",ClampMax="100")) float DialogueFOV=60.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NPC|Camera",meta=(ClampMin="0.05",ClampMax="1")) float FadeSeconds=.2f;
private:
 UPROPERTY(VisibleAnywhere) TObjectPtr<UWidgetComponent> Prompt;
 void ApplyAppearance();
 void UpdateNameplateRange();
};
