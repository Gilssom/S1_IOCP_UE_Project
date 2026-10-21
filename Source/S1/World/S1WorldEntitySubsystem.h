#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "S1WorldEntitySubsystem.generated.h"

class AS1Player;
class AS1DuskMinion;
class UParticleSystem;
class UNiagaraComponent;
class AS1LootActor;
class UDataTable;
struct FS1LootPresentation;

namespace Protocol
{
	class ObjectInfo;
	class S_MOVE;
	class S_COMBAT_ACTION;
	class S_COMBAT_ACTION_RESULT;
	class S_MONSTER_UPDATE;
	class S_MONSTER_ATTACK;
	class S_DAMAGE_RESULT;
	class S_PLAYER_STATUS;
	class S_PLAYER_HEALED;
}

UCLASS()
class S1_API US1WorldEntitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	friend class FS1KnightQApprovalTest;
	friend class FS1PotionHealingTest;
	friend class FS1MultiMonsterPresentationTest;

public:
	US1WorldEntitySubsystem();
	virtual void Deinitialize() override;

	void ConfigureOtherPlayerClass(
		TSubclassOf<AS1Player> InOtherPlayerClass);
	void ResetRegistry();
	void ResetLocalCombatPrediction(bool bResetSequence);
	void ResetCombatImpactState();
	void ResetLootRegistry();
	bool SyncLootPresentation(const TArray<FS1LootPresentation>& Drops);
	void UpdateLootHints(uint64 CharacterId,const FString& Selected,bool bPending);

	bool SpawnPlayer(
		const Protocol::ObjectInfo& ObjectInfo,
		bool bIsMine);
	bool SpawnMonster(const Protocol::ObjectInfo& ObjectInfo);
	bool DespawnPlayer(uint64 ObjectId);
	bool DespawnEntity(uint64 ObjectId);
	bool ApplyMove(const Protocol::S_MOVE& MovePacket);
	bool ApplyCombatAction(const Protocol::S_COMBAT_ACTION& CombatPacket);
	bool ApplyCombatActionResult(const Protocol::S_COMBAT_ACTION_RESULT& Packet);
	bool ApplyMonsterUpdate(const Protocol::S_MONSTER_UPDATE& Packet);
	bool ApplyMonsterAttack(const Protocol::S_MONSTER_ATTACK& Packet);
	bool ApplyDamageResult(const Protocol::S_DAMAGE_RESULT& Packet);
	bool ApplyPlayerStatus(const Protocol::S_PLAYER_STATUS& Packet);
	bool ApplyPlayerHealed(const Protocol::S_PLAYER_HEALED& Packet);

	bool ContainsPlayer(uint64 ObjectId) const;
	bool ContainsEntity(uint64 ObjectId) const;
	bool HasLocalPlayer() const;

private:
	struct FAcceptedAction
	{
		uint64 SourceObjectId = 0;
		int32 ActionType = 0;
		double ReceivedAtSeconds = 0.0;
	};

	void RememberAcceptedAction(uint64 ActionId, uint64 SourceObjectId, int32 ActionType);
	void PruneAcceptedActions();
	void PreloadImpactParticles();
	void TryPlayCombatImpact(const Protocol::S_DAMAGE_RESULT& Packet, AS1DuskMinion* Target);
	void ClearHealingEffects(uint64 ObjectId);
	bool AcceptHealthEvent(uint64 ObjectId,uint32 Sequence);

	// Hard CDO references: native-only soft paths were not collected by the cooker.
	UPROPERTY()
	TObjectPtr<UDataTable> MonsterDefinitions;
	UPROPERTY()
	TObjectPtr<UDataTable> MonsterPresentations;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UParticleSystem>> LoadedImpactParticles;

	TMap<uint64, FAcceptedAction> AcceptedActions;
	TSet<uint32> RecentImpactEventSequences;
	TArray<uint32> RecentImpactEventOrder;
	TSet<FString> RecentDamageKeys;
	TArray<FString> RecentDamageKeyOrder;
	TMap<uint64, uint32> LastDamageEvents;
	TSet<uint64> DeadPlayers;
	TSet<FString> RecentHealIds;
	TArray<FString> HealIdOrder;
	TMap<uint64,TArray<TWeakObjectPtr<UNiagaraComponent>>> HealingEffects;

	UPROPERTY(Transient)
	TSubclassOf<AS1Player> OtherPlayerClass;

	UPROPERTY(Transient)
	TObjectPtr<AS1Player> MyPlayer = nullptr;

	UPROPERTY(Transient)
	TMap<uint64, TObjectPtr<AS1Player>> Players;

	UPROPERTY(Transient)
	TMap<uint64, TObjectPtr<AS1DuskMinion>> Monsters;
	UPROPERTY(Transient)
	TMap<FString,TObjectPtr<AS1LootActor>> LootActors;
};
