#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Quest/S1QuestData.h"
#include "S1DialogueSubsystem.generated.h"
class AS1QuestNpc;
class AS1MyPlayer;
class ACameraActor;
class APlayerController;
class US1DialogueWidget;
class USkeletalMeshComponent;
class US1EntityHealthComponent;

/** Local interaction arbitration and reversible dialogue presentation, no quest authority. */
UCLASS()
class S1_API US1DialogueSubsystem : public UTickableWorldSubsystem
{
 GENERATED_BODY()
 friend class FS1DialogueTest;
public:
 virtual bool DoesSupportWorldType(EWorldType::Type Type) const override;
 virtual void Tick(float Delta) override;
 virtual TStatId GetStatId() const override;
 virtual void Deinitialize() override;
 void Interact();
 void CloseDialogue();
 void AbortDialogue();
 void NotifyLocalDamage();
 bool IsModal() const {return Phase!=EPhase::Closed;}
 bool ShouldShowLoot(const FString& Key) const;
 static bool PreferNpc(float NpcDistanceSquared,float LootDistanceSquared,bool bRetainNpc);
 static FVector TwoShotFocus(const FVector& PlayerFeet,const FVector& NpcFeet,float Height);
 static FVector PlayerSideCameraOffset(const FVector& PlayerFeet,const FVector& NpcFeet,const FVector& Offset,bool bMirrorShoulder);
private:
 enum class EPhase:uint8 {Closed,FadeToDialogue,RevealDialogue,Active,FadeToGame,RevealGame};
 EPhase Phase=EPhase::Closed;
 float PhaseTime=0,FadeDuration=.2f;
 double LastDamageAt=-1000;
 bool bCameraChanged=false,bSavedCursor=false;
 FRotator SavedControlRotation;
 TWeakObjectPtr<APlayerController> OwnerPC;
 TWeakObjectPtr<AS1MyPlayer> OwnerPawn;
 TWeakObjectPtr<AActor> SavedViewTarget;
 TWeakObjectPtr<AS1QuestNpc> CandidateNpc,ActiveNpc;
 FString SelectedLoot;
 UPROPERTY(Transient) TObjectPtr<ACameraActor> DialogueCamera;
 UPROPERTY(Transient) TObjectPtr<US1DialogueWidget> DialogueWidget;
 TWeakObjectPtr<USkeletalMeshComponent> TurnedNpcMesh;
 FRotator SavedNpcMeshRotation;
 bool bPrivatePresentation=false;
 TArray<TWeakObjectPtr<AActor>> AddedHiddenActors;
 TArray<TWeakObjectPtr<US1EntityHealthComponent>> SuppressedHealth;
 FS1DialogueSession DataSession;
 uint64 DialogueRun=0;
 void RefreshDialogueNode();
 void PrimaryDialogueAction();
 void SecondaryDialogueAction();
 AS1MyPlayer* LocalPlayer() const;
 bool IsAvailable(AS1MyPlayer* Player) const;
 bool HasCombatThreat() const;
 bool CanTalk(AS1MyPlayer* Player,AS1QuestNpc* Npc,float Radius) const;
 void RefreshCandidates();
 void BeginDialogue(AS1QuestNpc* Npc);
 void RevealDialogue();
 static void ConfigureDialogueCamera(ACameraActor* Camera,float FOV);
 void ApplyPrivatePresentation();
 void RestorePrivatePresentation();
 void RestoreView();
 void FinishDialogue();
};
