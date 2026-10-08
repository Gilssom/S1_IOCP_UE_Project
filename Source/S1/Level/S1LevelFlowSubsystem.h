#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Loot/S1LootState.h"
#include "S1LevelFlowSubsystem.generated.h"

namespace Protocol
{
	class ObjectInfo;
	class S_MOVE;
	class S_MONSTER_UPDATE;
	class S_DAMAGE_RESULT;
}

UENUM(BlueprintType)
enum class ES1LevelPhase : uint8
{
	Lobby,
	Loading,
	Main
};

struct FS1PendingEntitySpawn
{
	uint64 ObjectId = 0;
	FVector Location = FVector::ZeroVector;
	float Yaw = 0.f;
	int32 MoveState = 0;
	bool bIsMine = false;
	int32 CreatureType = 0;
	uint32 MonsterDefinitionId = 0;
	uint32 CurrentHp = 0;
	uint32 MaxHp = 0;
	int32 MonsterState = 0;
};

UCLASS()
class S1_API US1LevelFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void EnterLobbyState();
	void BeginMainLevelTravel();
	void OpenMainLevel();
	void CompleteMainLevelEntry();
	void TravelToLobby();

	ES1LevelPhase GetLevelPhase() const
	{
		return LevelPhase;
	}

	bool IsMainLevelReady() const
	{
		return bMainLevelReady;
	}

	void QueuePendingSpawn(
		const Protocol::ObjectInfo& ObjectInfo,
		bool bIsMine);
	bool UpdatePendingMove(const Protocol::S_MOVE& MovePacket);
	bool UpdatePendingMonster(const Protocol::S_MONSTER_UPDATE& UpdatePacket);
	bool UpdatePendingDamage(const Protocol::S_DAMAGE_RESULT& DamagePacket);
	void RemovePendingSpawn(uint64 ObjectId);
	TArray<FS1PendingEntitySpawn> GetPendingSpawnsOrdered() const;
	FS1LootState& GetLootState() {return LootState;}

private:
	ES1LevelPhase LevelPhase = ES1LevelPhase::Lobby;
	bool bMainLevelReady = false;
	TMap<uint64, FS1PendingEntitySpawn> PendingEntitySpawns;
	FS1LootState LootState;

	static const FName LobbyLevelName;
	static const FName LoadingLevelName;
	static const FName MainLevelName;
};
