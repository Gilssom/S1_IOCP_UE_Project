// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "S1.h"
#include "Account/S1AccountSubsystem.h"
#include "Level/S1LevelFlowSubsystem.h"
#include "Protocol.pb.h"
#include "S1GameInstance.generated.h"

class AS1Player;
class US1AuthWidget;
class US1CharacterSelectWidget;
class US1NetworkSubsystem;
class US1WorldEntitySubsystem;
enum class ES1ConnectionState : uint8;

/**
 * 
 */
UCLASS()
class S1_API US1GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	void EnterLobbyState();
	void BeginMainLevelTravel();
	void OpenMainLevel();
	void CompleteMainLevelEntry();

	UFUNCTION(BlueprintCallable)
	void TravelToLobby();

	UFUNCTION(BlueprintPure)
	ES1LevelPhase GetLevelPhase() const;

	UFUNCTION(BlueprintCallable)
	void ConnectToGameServer();

	UFUNCTION(BlueprintCallable)
	void Login(const FString& AccountName, const FString& Password);

	// Native authentication UI entry point. Legacy Blueprint Login nodes are
	// intentionally not allowed to authenticate without this UI gesture.
	void LoginFromAuthUI(
		const FString& AccountName,
		const FString& Password);

	UFUNCTION(BlueprintCallable)
	void SignUp(const FString& AccountName, const FString& Password);

	UFUNCTION(BlueprintCallable)
	void ShowAuthUI();

	UFUNCTION(BlueprintCallable)
	void HideAuthUI();

	void ShowCharacterSelectUI();
	void HideCharacterSelectUI();

	UFUNCTION(BlueprintPure)
	bool IsConnectedToGameServer() const;

	UFUNCTION(BlueprintCallable)
	void DisConnectFromGameServer();

	UFUNCTION(BlueprintCallable)
	void HandleRecvPackets();

	UFUNCTION(BlueprintCallable)
	void CreateCharacter(
		const FString& CharacterName,
		int32 PlayerType);

	UFUNCTION(BlueprintCallable)
	void EnterCharacter(int64 CharacterId);

	UFUNCTION(BlueprintCallable)
	void EnterFirstCharacter();

	const TArray<FS1CharacterSummary>& GetCharacters() const;

	void SendPacket(SendBufferRef SendBuffer);
	bool SupportsCombatApproval() const { return CombatContractVersion >= 1; }
	bool SupportsKnightQApproval() const { return CombatContractVersion >= 2; }

public:
	void HandleSpawn(const Protocol::ObjectInfo& ObjectInfo, bool IsMine);
	void HandleSpawn(const Protocol::S_ENTER_GAME& EnterGamePkt);
	void HandleSpawn(const Protocol::S_SPAWN& SpawnPkt);

	void HandleDeSpawn(uint64 ObjectId);
	void HandleDeSpawn(const Protocol::S_DESPAWN& DeSpawnPkt);

	void HandleMove(const Protocol::S_MOVE& MovePkt);

private:
	void HandleEnterGamePacket(const Protocol::S_ENTER_GAME& Packet);
	void HandleLeaveGamePacket(const Protocol::S_LEAVE_GAME& Packet);
	void HandleSpawnPacket(const Protocol::S_SPAWN& Packet);
	void HandleDespawnPacket(const Protocol::S_DESPAWN& Packet);
	void HandleMovePacket(const Protocol::S_MOVE& Packet);
	void HandleChatPacket(const Protocol::S_CHAT& Packet);
	void HandleCombatActionPacket(const Protocol::S_COMBAT_ACTION& Packet);
	void HandleCombatActionResultPacket(const Protocol::S_COMBAT_ACTION_RESULT& Packet);
	void HandleCombatConnectionState(ES1ConnectionState State, const FString& Reason);
	void HandleMonsterUpdatePacket(const Protocol::S_MONSTER_UPDATE& Packet);
	void HandleMonsterAttackPacket(const Protocol::S_MONSTER_ATTACK& Packet);
	void HandleDamageResultPacket(const Protocol::S_DAMAGE_RESULT& Packet);
	void HandlePlayerStatusPacket(const Protocol::S_PLAYER_STATUS& Packet);
	void HandlePlayerHealedPacket(const Protocol::S_PLAYER_HEALED& Packet);
	void HandleLootSnapshotPacket(const Protocol::S_LOOT_SNAPSHOT& Packet);
	void HandleLootCreatedPacket(const Protocol::S_LOOT_CREATED& Packet);
	void HandleLootRemovedPacket(const Protocol::S_LOOT_REMOVED& Packet);
	void FlushLootPresentation();

	UFUNCTION()
	void HandleAccountLoginCompleted(
		bool bSuccess,
		const FString& Message);

	UFUNCTION()
	void HandleAccountSignUpCompleted(
		bool bSuccess,
		int32 ResultCode,
		const FString& Message);

	UFUNCTION()
	void HandleAccountCharacterCreated(
		bool bSuccess,
		const FString& Message);

	UFUNCTION()
	void HandleAccountEnterGameCompleted(
		bool bSuccess,
		const FString& Message);

	void FlushPendingSpawns();
	bool SpawnPendingEntity(const FS1PendingEntitySpawn& PendingSpawn);
	US1WorldEntitySubsystem* GetWorldEntitySubsystem() const;

public:
	UPROPERTY(BlueprintAssignable)
	FOnLoginCompleted OnLoginCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnSignUpCompleted OnSignUpCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnCharacterCreated OnCharacterCreated;

	UPROPERTY(BlueprintAssignable)
	FOnEnterGameCompleted OnEnterGameCompleted;

	UPROPERTY(Transient)
	TObjectPtr<US1AuthWidget> AuthWidget;

	UPROPERTY(Transient)
	TObjectPtr<US1CharacterSelectWidget> CharacterSelectWidget;

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AS1Player> OtherPlayerClass;

private:
	uint32 CombatContractVersion = 0;
};
