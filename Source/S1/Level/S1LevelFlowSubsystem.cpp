#include "Level/S1LevelFlowSubsystem.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Protocol.pb.h"
#include "Level/S1RegionContract.h"

const FName US1LevelFlowSubsystem::LobbyLevelName(
	TEXT("/Game/Maps/LobbyMap"));
const FName US1LevelFlowSubsystem::LoadingLevelName(
	TEXT("/Game/Maps/LoadingMap"));
bool US1LevelFlowSubsystem::SetInitialRegion(uint32 RegionId)
{
	if(LevelPhase!=ES1LevelPhase::Lobby||FS1RegionContract::MapFor(RegionId).IsNone())return false;
	InitialRegionId=RegionId;return true;
}

void US1LevelFlowSubsystem::EnterLobbyState()
{
	LevelPhase = ES1LevelPhase::Lobby;
	bMainLevelReady = false;
	PendingEntitySpawns.Reset();
	LootState.Reset();
	InitialRegionId = 0;
}

void US1LevelFlowSubsystem::BeginMainLevelTravel()
{
	if (LevelPhase != ES1LevelPhase::Lobby || FS1RegionContract::MapFor(InitialRegionId).IsNone())
		return;

	LevelPhase = ES1LevelPhase::Loading;
	bMainLevelReady = false;
	UGameplayStatics::OpenLevel(
		GetGameInstance(),
		LoadingLevelName);
}

void US1LevelFlowSubsystem::OpenMainLevel()
{
	if (LevelPhase != ES1LevelPhase::Loading)
		return;
	const FName Destination=FS1RegionContract::MapFor(InitialRegionId);
	if(Destination.IsNone())return;

	UGameplayStatics::OpenLevel(
		GetGameInstance(),
		Destination);
}

void US1LevelFlowSubsystem::CompleteMainLevelEntry()
{
	LevelPhase = ES1LevelPhase::Main;
	bMainLevelReady = true;
}

void US1LevelFlowSubsystem::TravelToLobby()
{
	EnterLobbyState();
	UGameplayStatics::OpenLevel(
		GetGameInstance(),
		LobbyLevelName);
}

void US1LevelFlowSubsystem::QueuePendingSpawn(
	const Protocol::ObjectInfo& ObjectInfo,
	bool bIsMine)
{
	FS1PendingEntitySpawn& Pending =
		PendingEntitySpawns.FindOrAdd(ObjectInfo.object_id());
	Pending.ObjectId = ObjectInfo.object_id();
	Pending.Location = FVector(
		ObjectInfo.pos_info().x(),
		ObjectInfo.pos_info().y(),
		ObjectInfo.pos_info().z());
	Pending.Yaw = ObjectInfo.pos_info().yaw();
	Pending.MoveState = static_cast<int32>(
		ObjectInfo.pos_info().state());
	Pending.bIsMine = Pending.bIsMine || bIsMine;
	Pending.CreatureType = static_cast<int32>(ObjectInfo.creature_type());
	Pending.MonsterDefinitionId = ObjectInfo.monster_definition_id();
	Pending.CurrentHp = ObjectInfo.current_hp();
	Pending.MaxHp = ObjectInfo.max_hp();
	Pending.MonsterState = static_cast<int32>(ObjectInfo.monster_state());
}

bool US1LevelFlowSubsystem::UpdatePendingMove(
	const Protocol::S_MOVE& MovePacket)
{
	FS1PendingEntitySpawn* Pending =
		PendingEntitySpawns.Find(MovePacket.info().object_id());
	if (Pending == nullptr)
		return false;

	Pending->Location = FVector(
		MovePacket.info().x(),
		MovePacket.info().y(),
		MovePacket.info().z());
	Pending->Yaw = MovePacket.info().yaw();
	Pending->MoveState = static_cast<int32>(
		MovePacket.info().state());
	return true;
}

bool US1LevelFlowSubsystem::UpdatePendingMonster(
	const Protocol::S_MONSTER_UPDATE& UpdatePacket)
{
	FS1PendingEntitySpawn* Pending = PendingEntitySpawns.Find(UpdatePacket.object_id());
	if (Pending == nullptr)
	{
		return false;
	}
	Pending->Location = FVector(UpdatePacket.pos_info().x(), UpdatePacket.pos_info().y(),
		UpdatePacket.pos_info().z());
	Pending->Yaw = UpdatePacket.pos_info().yaw();
	Pending->CurrentHp = UpdatePacket.current_hp();
	Pending->MaxHp = UpdatePacket.max_hp();
	Pending->MonsterState = static_cast<int32>(UpdatePacket.state());
	return true;
}

bool US1LevelFlowSubsystem::UpdatePendingDamage(
	const Protocol::S_DAMAGE_RESULT& DamagePacket)
{
	FS1PendingEntitySpawn* Pending = PendingEntitySpawns.Find(DamagePacket.target_object_id());
	if (Pending == nullptr)
	{
		return false;
	}
	Pending->CurrentHp = DamagePacket.current_hp();
	Pending->MaxHp = DamagePacket.max_hp();
	return true;
}

void US1LevelFlowSubsystem::RemovePendingSpawn(uint64 ObjectId)
{
	PendingEntitySpawns.Remove(ObjectId);
}

TArray<FS1PendingEntitySpawn>
US1LevelFlowSubsystem::GetPendingSpawnsOrdered() const
{
	TArray<FS1PendingEntitySpawn> Result;
	PendingEntitySpawns.GenerateValueArray(Result);
	Result.Sort([](
		const FS1PendingEntitySpawn& Left,
		const FS1PendingEntitySpawn& Right)
	{
		if (Left.bIsMine != Right.bIsMine)
			return Left.bIsMine;

		return Left.ObjectId < Right.ObjectId;
	});
	return Result;
}
