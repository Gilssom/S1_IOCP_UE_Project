#include "World/S1WorldEntitySubsystem.h"

#include "Combat/S1KnightCombatComponent.h"
#include "Combat/S1CombatImpactSettings.h"
#include "Game/S1Player.h"
#include "Game/S1MyPlayer.h"
#include "Game/S1DuskMinion.h"
#include "Game/S1MonsterData.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Protocol.pb.h"
#include "TimerManager.h"
#include "Loot/S1LootActor.h"
#include "Loot/S1LootState.h"
#include "Loot/S1PotionPresentationSettings.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Quest/S1DialogueSubsystem.h"

US1WorldEntitySubsystem::US1WorldEntitySubsystem()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> Definitions(TEXT("/Game/S1/Monsters/Data/DT_S1MonsterDefinitions.DT_S1MonsterDefinitions"));
	static ConstructorHelpers::FObjectFinder<UDataTable> Presentations(TEXT("/Game/S1/Monsters/Data/DT_S1MonsterPresentations.DT_S1MonsterPresentations"));
	MonsterDefinitions=Definitions.Object;MonsterPresentations=Presentations.Object;
}

void US1WorldEntitySubsystem::Deinitialize()
{
	ResetRegistry();
	Super::Deinitialize();
}

void US1WorldEntitySubsystem::ConfigureOtherPlayerClass(
	TSubclassOf<AS1Player> InOtherPlayerClass)
{
	OtherPlayerClass = InOtherPlayerClass;
}

void US1WorldEntitySubsystem::ResetRegistry()
{
	if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>())Dialogue->AbortDialogue();
	TArray<uint64> HealingIds;HealingEffects.GenerateKeyArray(HealingIds);
	for(uint64 Id:HealingIds)ClearHealingEffects(Id);
	DeadPlayers.Reset();RecentHealIds.Reset();HealIdOrder.Reset();
	ResetLocalCombatPrediction(false);
	ResetCombatImpactState();
	ResetLootRegistry();
	for (const auto& Entry : Monsters)
	{
		if (IsValid(Entry.Value))
		{
			Entry.Value->Destroy();
		}
	}
	Monsters.Reset();
	MyPlayer = nullptr;
	Players.Reset();
}

void US1WorldEntitySubsystem::ResetCombatImpactState()
{
	AcceptedActions.Reset();
	RecentImpactEventSequences.Reset();
	RecentImpactEventOrder.Reset();
	LoadedImpactParticles.Reset();
	RecentDamageKeys.Reset();
	RecentDamageKeyOrder.Reset();
	LastDamageEvents.Reset();
}

void US1WorldEntitySubsystem::ResetLootRegistry()
{
 for(auto& Entry:LootActors)
  if(IsValid(Entry.Value)){Entry.Value->RetirePresentation();Entry.Value->Destroy();}
 LootActors.Reset();
}

bool US1WorldEntitySubsystem::SyncLootPresentation(const TArray<FS1LootPresentation>& Drops)
{
 check(IsInGameThread());
 TSet<FString> Active;bool bSuccess=true;
 for(const auto& Drop:Drops)
 {
  Active.Add(Drop.Key);
  AS1LootActor* Actor=LootActors.FindRef(Drop.Key);
  if(!IsValid(Actor))
  {
   Actor=GetWorld()->SpawnActor<AS1LootActor>();
   if(!Actor){bSuccess=false;continue;}
   if(!Actor->ApplyServerDrop(Drop.Key,Drop.OwnerCharacterId,Drop.Count,Drop.Floor,Drop.SourceKind==2))
   {Actor->Destroy();bSuccess=false;continue;}
   LootActors.Add(Drop.Key,Actor);
   UE_LOG(LogTemp,Log,TEXT("S1 loot presentation spawned. DropId=%s Floor=%s"),*Drop.Key,*Drop.Floor.ToString());
  }
 }
 for(auto It=LootActors.CreateIterator();It;++It)
  if(!Active.Contains(It.Key()))
  {
   if(IsValid(It.Value())){It.Value()->RetirePresentation();It.Value()->Destroy();}
   UE_LOG(LogTemp,Log,TEXT("S1 loot presentation removed. DropId=%s"),*It.Key());
   It.RemoveCurrent();
  }
 return bSuccess;
}

void US1WorldEntitySubsystem::ResetLocalCombatPrediction(bool bResetSequence)
{
	if (AS1MyPlayer* Local = Cast<AS1MyPlayer>(MyPlayer.Get()))
		Local->ResetPredictedCombat(bResetSequence);
}

bool US1WorldEntitySubsystem::SpawnMonster(const Protocol::ObjectInfo& ObjectInfo)
{
	if (ObjectInfo.creature_type() != Protocol::CREATURE_TYPE_MONSTER ||
		ObjectInfo.monster_definition_id() == 0 || ObjectInfo.monster_definition_id() > 1000000 || ObjectInfo.object_id() == 0)
	{
		return false;
	}
	if (Monsters.Contains(ObjectInfo.object_id()))
	{
		return true;
	}
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const FVector Location(ObjectInfo.pos_info().x(), ObjectInfo.pos_info().y(),
		ObjectInfo.pos_info().z());
	const float Yaw = ObjectInfo.pos_info().yaw();
	FActorSpawnParameters Parameters;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const auto* Definitions=MonsterDefinitions.Get();
	const auto* Presentations=MonsterPresentations.Get();
	FString Error;
	const auto* Definition=FS1MonsterData::FindDefinition(Definitions,ObjectInfo.monster_definition_id());
	const auto* Presentation=FS1MonsterData::FindPresentation(Presentations,ObjectInfo.monster_definition_id());
	if(!FS1MonsterData::Validate(Definitions,Presentations,Error)||!Definition||!Presentation)
	{UE_LOG(LogTemp,Warning,TEXT("Monster presentation rejected. Definition[%u] Reason[%s]"),ObjectInfo.monster_definition_id(),Error.IsEmpty()?TEXT("Unregistered monster ID"):*Error);return false;}
	AS1DuskMinion* Monster = World->SpawnActor<AS1DuskMinion>(
		Presentation->ActorClass, Location, FRotator(0.f, Yaw, 0.f), Parameters);
	if (Monster == nullptr)
	{
		return false;
	}
	Monster->ConfigureDefinition(Presentation->DisplayName,Definition->CapsuleRadius,Definition->CapsuleHalfHeight);
	Monster->ApplyServerTransform(Location, Yaw);
	Monster->ApplyInitialServerState(static_cast<int32>(ObjectInfo.monster_state()),
		ObjectInfo.current_hp(), ObjectInfo.max_hp());
	Monsters.Add(ObjectInfo.object_id(), Monster);
	PreloadImpactParticles();
	UE_LOG(LogTemp, Log, TEXT("Dusk spawned. ObjectId[%llu], Hp[%u/%u]"),
		ObjectInfo.object_id(), ObjectInfo.current_hp(), ObjectInfo.max_hp());
	return true;
}

bool US1WorldEntitySubsystem::SpawnPlayer(
	const Protocol::ObjectInfo& ObjectInfo,
	bool bIsMine)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
		return false;

	const uint64 ObjectId = ObjectInfo.object_id();
	if (Players.Contains(ObjectId))
		return true;

	const FVector SpawnLocation(
		ObjectInfo.pos_info().x(),
		ObjectInfo.pos_info().y(),
		ObjectInfo.pos_info().z());

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"World entity spawn. ObjectId[%llu], IsMine[%s], "
			"Position[%s]"),
		ObjectId,
		bIsMine ? TEXT("true") : TEXT("false"),
		*SpawnLocation.ToString());

	if (bIsMine)
	{
		APlayerController* PlayerController =
			UGameplayStatics::GetPlayerController(World, 0);
		AS1Player* Player = PlayerController != nullptr
			? Cast<AS1Player>(PlayerController->GetPawn())
			: nullptr;
		if (Player == nullptr)
			return false;

		Player->SetPlayerInfo(ObjectInfo.pos_info());
		Player->ApplyServerHealth(ObjectInfo.current_hp(), ObjectInfo.max_hp(),
			ObjectInfo.current_hp() == 0);
		Player->SetActorLocationAndRotation(
			SpawnLocation,
			FRotator(0.f, ObjectInfo.pos_info().yaw(), 0.f));

		MyPlayer = Player;
		Players.Add(ObjectId, Player);
		if(ObjectInfo.max_hp()>0&&ObjectInfo.current_hp()==0)DeadPlayers.Add(ObjectId);
		return true;
	}

	if (OtherPlayerClass == nullptr)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("OtherPlayerClass is not configured. ObjectId[%llu]"),
			ObjectId);
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AS1Player* OtherPlayer = World->SpawnActor<AS1Player>(
		OtherPlayerClass,
		SpawnLocation,
		FRotator(0.f, ObjectInfo.pos_info().yaw(), 0.f),
		SpawnParameters);
	if (OtherPlayer == nullptr)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Other player spawn failed. ObjectId[%llu], "
				"Position[%s]"),
			ObjectId,
			*SpawnLocation.ToString());
		return false;
	}

	OtherPlayer->SetPlayerInfo(ObjectInfo.pos_info());
	OtherPlayer->ApplyServerHealth(ObjectInfo.current_hp(), ObjectInfo.max_hp(),
		ObjectInfo.current_hp() == 0);
	Players.Add(ObjectId, OtherPlayer);
	return true;
}

bool US1WorldEntitySubsystem::DespawnPlayer(uint64 ObjectId)
{
	ClearHealingEffects(ObjectId);DeadPlayers.Remove(ObjectId);LastDamageEvents.Remove(ObjectId);
	TObjectPtr<AS1Player>* FoundPlayer = Players.Find(ObjectId);
	if (FoundPlayer == nullptr)
		return false;

	AS1Player* Player = FoundPlayer->Get();
	Players.Remove(ObjectId);

	if (Player == MyPlayer)
		MyPlayer = nullptr;

	if (IsValid(Player))
		Player->Destroy();

	return true;
}

bool US1WorldEntitySubsystem::DespawnEntity(uint64 ObjectId)
{
	ClearHealingEffects(ObjectId);DeadPlayers.Remove(ObjectId);
	LastDamageEvents.Remove(ObjectId);
	for (auto It = AcceptedActions.CreateIterator(); It; ++It)
	{
		if (It.Value().SourceObjectId == ObjectId)
			It.RemoveCurrent();
	}
	if (TObjectPtr<AS1DuskMinion>* Found = Monsters.Find(ObjectId))
	{
		if (IsValid(Found->Get()))
		{
			Found->Get()->Destroy();
		}
		Monsters.Remove(ObjectId);
		return true;
	}
	return DespawnPlayer(ObjectId);
}

bool US1WorldEntitySubsystem::ApplyMonsterUpdate(const Protocol::S_MONSTER_UPDATE& Packet)
{
	TObjectPtr<AS1DuskMinion>* Found = Monsters.Find(Packet.object_id());
	if (Found == nullptr || !IsValid(Found->Get()))
	{
		return false;
	}
	AS1DuskMinion* Monster = Found->Get();
	Monster->ApplyServerTransform(FVector(Packet.pos_info().x(), Packet.pos_info().y(),
		Packet.pos_info().z()), Packet.pos_info().yaw());
	Monster->ApplyServerState(static_cast<int32>(Packet.state()), Packet.current_hp(),
		Packet.max_hp());
	Monster->SetLocalCombatTarget(MyPlayer && Packet.target_object_id()==MyPlayer->GetPlayerInfo()->object_id()
		&& (Packet.state()==Protocol::MONSTER_STATE_CHASE || Packet.state()==Protocol::MONSTER_STATE_ATTACK));
	return true;
}

bool US1WorldEntitySubsystem::ApplyMonsterAttack(const Protocol::S_MONSTER_ATTACK& Packet)
{
	TObjectPtr<AS1DuskMinion>* Found = Monsters.Find(Packet.object_id());
	if (Found == nullptr || !IsValid(Found->Get()))
	{
		return false;
	}
	Found->Get()->PlayServerAttack(Packet.action_instance_id(), Packet.facing_yaw());
	Found->Get()->SetLocalCombatTarget(MyPlayer && Packet.target_object_id()==MyPlayer->GetPlayerInfo()->object_id());
	return true;
}

bool US1WorldEntitySubsystem::ApplyDamageResult(const Protocol::S_DAMAGE_RESULT& Packet)
{
	if (!ContainsEntity(Packet.target_object_id()) || Packet.action_instance_id() == 0 ||
		Packet.room_event_sequence() == 0)
		return false;
	const FString DamageKey = FString::Printf(TEXT("%llu:%llu:%llu:%u"),
		Packet.source_object_id(), Packet.action_instance_id(), Packet.target_object_id(), Packet.hit_index());
	if(Packet.max_hp()==0||Packet.current_hp()>Packet.max_hp()||
	 (DeadPlayers.Contains(Packet.target_object_id())&&Packet.current_hp()>0)||RecentDamageKeys.Contains(DamageKey)||
	 !AcceptHealthEvent(Packet.target_object_id(),Packet.room_event_sequence()))
		return false;
	RecentDamageKeys.Add(DamageKey);
	RecentDamageKeyOrder.Add(DamageKey);
	if (RecentDamageKeyOrder.Num() > 256)
	{
		RecentDamageKeys.Remove(RecentDamageKeyOrder[0]);
		RecentDamageKeyOrder.RemoveAt(0);
	}
	UE_LOG(LogTemp, Log, TEXT("Combat damage applied. Action[%llu], Hit[%u], Source[%llu], Target[%llu], Event[%u], Damage[%u], Hp[%u/%u]"),
		Packet.action_instance_id(), Packet.hit_index(), Packet.source_object_id(), Packet.target_object_id(),
		Packet.room_event_sequence(), Packet.damage(), Packet.current_hp(), Packet.max_hp());
	if (TObjectPtr<AS1DuskMinion>* Found = Monsters.Find(Packet.target_object_id()))
	{
		if (IsValid(Found->Get()))
		{
			Found->Get()->ApplyServerHealth(Packet.current_hp(), Packet.max_hp());
			if(MyPlayer && Packet.source_object_id()==MyPlayer->GetPlayerInfo()->object_id())Found->Get()->NotifyLocalCombatHit();
			TryPlayCombatImpact(Packet, Found->Get());
			return true;
		}
	}
	if (Players.Contains(Packet.target_object_id()))
	{
		if(MyPlayer && Packet.target_object_id()==MyPlayer->GetPlayerInfo()->object_id())
			if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>())Dialogue->NotifyLocalDamage();
		if(MyPlayer && Packet.target_object_id()==MyPlayer->GetPlayerInfo()->object_id())
			if(auto* Source=Monsters.Find(Packet.source_object_id());Source && IsValid(Source->Get()))Source->Get()->NotifyLocalCombatHit();
		if(Packet.current_hp()==0){DeadPlayers.Add(Packet.target_object_id());ClearHealingEffects(Packet.target_object_id());}
		if (AS1Player* Player = Players[Packet.target_object_id()].Get())
		{
			Player->ApplyServerHealth(Packet.current_hp(), Packet.max_hp(),
				Packet.current_hp() == 0);
		}
		UE_LOG(LogTemp, Log, TEXT("Server player damage. Target[%llu], Damage[%u], Hp[%u/%u]"),
			Packet.target_object_id(), Packet.damage(), Packet.current_hp(), Packet.max_hp());
		return true;
	}
	return false;
}

bool US1WorldEntitySubsystem::ApplyPlayerStatus(const Protocol::S_PLAYER_STATUS& Packet)
{
	TObjectPtr<AS1Player>* Found = Players.Find(Packet.object_id());
	if (Found == nullptr || !IsValid(Found->Get()))
	{
		return false;
	}
	if(!Packet.max_hp()||Packet.current_hp()>Packet.max_hp()||Packet.is_dead()!=(Packet.current_hp()==0)||
	 !AcceptHealthEvent(Packet.object_id(),Packet.room_event_sequence()))return false;
	AS1Player* Player = Found->Get();
	Player->ApplyServerHealth(Packet.current_hp(), Packet.max_hp(), Packet.is_dead());
	if (Packet.is_dead())
	{
		if(Player==MyPlayer)if(auto* Dialogue=GetWorld()->GetSubsystem<US1DialogueSubsystem>())Dialogue->AbortDialogue();
		DeadPlayers.Add(Packet.object_id());ClearHealingEffects(Packet.object_id());
		if (AS1MyPlayer* Local = Cast<AS1MyPlayer>(Player))
			Local->ResetPredictedCombat();
		Player->GetKnightCombatComponent()->InterruptByServer();
		Player->GetCharacterMovement()->DisableMovement();
	}
	else
	{
		DeadPlayers.Remove(Packet.object_id());
		if (Player == MyPlayer)
		{
			Player->SetActorLocation(FVector(Packet.pos_info().x(), Packet.pos_info().y(),
				Packet.pos_info().z()));
		}
		else
		{
			Player->SetDestInfo(Packet.pos_info());
		}
		Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	UE_LOG(LogTemp, Log, TEXT("Server player status. ObjectId[%llu], Dead[%s], Hp[%u/%u]"),
		Packet.object_id(), Packet.is_dead() ? TEXT("true") : TEXT("false"),
		Packet.current_hp(), Packet.max_hp());
	return true;
}

bool US1WorldEntitySubsystem::AcceptHealthEvent(uint64 ObjectId,uint32 Sequence)
{
 if(!Sequence)return false;
 const auto* Previous=LastDamageEvents.Find(ObjectId);const uint32 Delta=Previous?Sequence-*Previous:1;
 if(!Delta||Delta>=0x80000000u)return false;
 LastDamageEvents.Add(ObjectId,Sequence);return true;
}
void US1WorldEntitySubsystem::ClearHealingEffects(uint64 ObjectId)
{
 if(auto* Effects=HealingEffects.Find(ObjectId))
  for(const auto& Weak:*Effects)if(auto* Effect=Weak.Get())
  {Effect->DeactivateImmediate();if(auto* Owner=Effect->GetOwner())Owner->RemoveInstanceComponent(Effect);Effect->DestroyComponent();}
 HealingEffects.Remove(ObjectId);
}
bool US1WorldEntitySubsystem::ApplyPlayerHealed(const Protocol::S_PLAYER_HEALED& Packet)
{
 auto* Found=Players.Find(Packet.object_id());
 if(!Found||!IsValid(Found->Get())||DeadPlayers.Contains(Packet.object_id())||Packet.effect_event_id().size()!=16||
  Packet.effect_event_id().find_first_not_of('\0')==std::string::npos||!Packet.actual_heal()||Packet.actual_heal()>50||
  !Packet.current_hp()||Packet.current_hp()>Packet.max_hp()||Packet.actual_heal()>Packet.current_hp())return false;
 const FString Key=FString::FromHexBlob(reinterpret_cast<const uint8*>(Packet.effect_event_id().data()),16);
 if(RecentHealIds.Contains(Key)||!AcceptHealthEvent(Packet.object_id(),Packet.room_event_sequence()))return false;
 RecentHealIds.Add(Key);HealIdOrder.Add(Key);
 if(HealIdOrder.Num()>256){RecentHealIds.Remove(HealIdOrder[0]);HealIdOrder.RemoveAt(0);}
 auto* Player=Found->Get();Player->ApplyServerHealth(Packet.current_hp(),Packet.max_hp(),false);
 // HP only: never relocate, change movement mode, interrupt combat or infer FX from snapshots.
 UE_LOG(LogTemp,Log,TEXT("Server heal applied. Object[%llu] Event[%u] Effect[%s] Heal[%u] Hp[%u/%u]"),
  Packet.object_id(),Packet.room_event_sequence(),*Key,Packet.actual_heal(),Packet.current_hp(),Packet.max_hp());
 const auto* Settings=GetDefault<US1PotionPresentationSettings>();
 auto* System=Cast<UNiagaraSystem>(Settings->HealingEffect.TryLoad());
 if(!IsValid(System))return true; // Cosmetic failure never rejects authoritative health.
 auto* Effect=NewObject<UNiagaraComponent>(Player);
 Effect->SetAutoActivate(false);Effect->SetAutoDestroy(false);Effect->SetAsset(System);
 Effect->SetCollisionEnabled(ECollisionEnabled::NoCollision);Effect->SetCanEverAffectNavigation(false);
 Effect->SetupAttachment(Player->GetRootComponent());
 Effect->SetRelativeLocation(FVector(0,0,-Player->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())+Settings->HealingFootOffset);
 Effect->SetRelativeScale3D(FVector(FMath::Clamp(Settings->HealingScale,0.01f,10.f)));
 Player->AddInstanceComponent(Effect);Effect->RegisterComponent();Effect->Activate(true);
 auto& Active=HealingEffects.FindOrAdd(Packet.object_id());Active.RemoveAll([](const auto& Weak){return !Weak.IsValid();});Active.Add(Effect);
 const TWeakObjectPtr<UNiagaraComponent> WeakEffect=Effect;
 const float EmitSeconds=FMath::Clamp(Settings->HealingEmitSeconds,0.1f,10.f);
 const float MaxLife=FMath::Clamp(Settings->HealingMaxLifeSeconds,EmitSeconds+0.1f,15.f);
 FTimerHandle StopHandle,DestroyHandle;
 GetWorld()->GetTimerManager().SetTimer(StopHandle,FTimerDelegate::CreateLambda([WeakEffect]
 {if(auto* Component=WeakEffect.Get())Component->Deactivate();}),EmitSeconds,false);
 GetWorld()->GetTimerManager().SetTimer(DestroyHandle,FTimerDelegate::CreateLambda([WeakEffect]
 {if(auto* Component=WeakEffect.Get()){Component->DeactivateImmediate();if(auto* Owner=Component->GetOwner())Owner->RemoveInstanceComponent(Component);Component->DestroyComponent();}}),MaxLife,false);
 return true;
}

bool US1WorldEntitySubsystem::ApplyMove(
	const Protocol::S_MOVE& MovePacket)
{
	TObjectPtr<AS1Player>* FoundPlayer =
		Players.Find(MovePacket.info().object_id());
	if (FoundPlayer == nullptr)
		return false;

	AS1Player* Player = FoundPlayer->Get();
	if (!IsValid(Player) || Player == MyPlayer)
		return false;

	Player->SetDestInfo(MovePacket.info());
	return true;
}

bool US1WorldEntitySubsystem::ApplyCombatAction(
	const Protocol::S_COMBAT_ACTION& CombatPacket)
{
	TObjectPtr<AS1Player>* FoundPlayer =
		Players.Find(CombatPacket.object_id());
	if (FoundPlayer == nullptr)
	{
		return false;
	}
	AS1Player* Player = FoundPlayer->Get();
	if (!IsValid(Player) || Player == MyPlayer)
	{
		return false;
	}
	RememberAcceptedAction(CombatPacket.action_instance_id(),
		CombatPacket.object_id(), static_cast<int32>(CombatPacket.action_type()));

	ES1KnightCombatAction Action;
	switch (CombatPacket.action_type())
	{
	case Protocol::COMBAT_ACTION_KNIGHT_PRIMARY:
		Action = ES1KnightCombatAction::PrimaryCombo;
		break;
	case Protocol::COMBAT_ACTION_KNIGHT_Q:
		Action = ES1KnightCombatAction::Charge;
		break;
	default:
		return false;
	}

	US1KnightCombatComponent* Combat = Player->GetKnightCombatComponent();
	const bool bPlayed = Combat != nullptr && Combat->PlayReplicatedAction(
		Action,
		static_cast<int32>(CombatPacket.combo_step()),
		CombatPacket.sequence(),
		CombatPacket.facing_yaw());

	UE_LOG(LogTemp, Log,
		TEXT("Remote combat action %s. ObjectId[%llu], Type[%d], Step[%u], Sequence[%u], FacingYaw[%.2f]"),
		bPlayed ? TEXT("played") : TEXT("rejected"),
		CombatPacket.object_id(),
		static_cast<int32>(CombatPacket.action_type()),
		CombatPacket.combo_step(),
		CombatPacket.sequence(),
		CombatPacket.facing_yaw());
	return bPlayed;
}

bool US1WorldEntitySubsystem::ApplyCombatActionResult(
	const Protocol::S_COMBAT_ACTION_RESULT& Packet)
{
	if (AS1MyPlayer* Local = Cast<AS1MyPlayer>(MyPlayer.Get()))
	{
		if (Local->GetPlayerInfo() != nullptr &&
			Local->GetPlayerInfo()->object_id() == Packet.source_object_id())
		{
			Local->ApplyCombatActionResult(Packet);
			if (Packet.decision() == Protocol::COMBAT_DECISION_ACCEPTED)
			{
				RememberAcceptedAction(Packet.action_instance_id(),
					Packet.source_object_id(), static_cast<int32>(Packet.action_type()));
			}
			return true;
		}
	}
	return false;
}

void US1WorldEntitySubsystem::PruneAcceptedActions()
{
	const double Now = FPlatformTime::Seconds();
	for (auto It = AcceptedActions.CreateIterator(); It; ++It)
	{
		if (Now - It.Value().ReceivedAtSeconds > 8.0)
			It.RemoveCurrent();
	}
	while (AcceptedActions.Num() > 64)
	{
		uint64 OldestId = 0;
		double OldestTime = TNumericLimits<double>::Max();
		for (const auto& Entry : AcceptedActions)
		{
			if (Entry.Value.ReceivedAtSeconds < OldestTime)
			{
				OldestTime = Entry.Value.ReceivedAtSeconds;
				OldestId = Entry.Key;
			}
		}
		if (OldestId == 0)
			break;
		AcceptedActions.Remove(OldestId);
	}
}

void US1WorldEntitySubsystem::RememberAcceptedAction(
	uint64 ActionId, uint64 SourceObjectId, int32 ActionType)
{
	if (ActionId == 0 || SourceObjectId == 0 || ActionType <= 0)
		return;
	AcceptedActions.Add(ActionId,
		{ SourceObjectId, ActionType, FPlatformTime::Seconds() });
	PruneAcceptedActions();
}

void US1WorldEntitySubsystem::PreloadImpactParticles()
{
	const US1CombatImpactSettings* Settings = GetDefault<US1CombatImpactSettings>();
	for (const FS1CombatImpactRule& Rule : Settings->ImpactRules)
	{
		if (Rule.ActionType <= 0 || Rule.ParticleAsset.IsNull() ||
			LoadedImpactParticles.Contains(Rule.ActionType))
			continue;
		UParticleSystem* Particle = Cast<UParticleSystem>(Rule.ParticleAsset.TryLoad());
		if (Particle != nullptr)
		{
			LoadedImpactParticles.Add(Rule.ActionType, Particle);
			UE_LOG(LogTemp, Log, TEXT("Combat impact particle loaded. ActionType[%d], Path[%s]"),
				Rule.ActionType, *Rule.ParticleAsset.ToString());
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Combat impact particle unavailable. ActionType[%d], Path[%s]"),
				Rule.ActionType, *Rule.ParticleAsset.ToString());
	}
}

void US1WorldEntitySubsystem::TryPlayCombatImpact(
	const Protocol::S_DAMAGE_RESULT& Packet, AS1DuskMinion* Target)
{
	if (!IsValid(Target) || Packet.damage() == 0 ||
		Packet.action_instance_id() == 0 || Packet.room_event_sequence() == 0 ||
		RecentImpactEventSequences.Contains(Packet.room_event_sequence()))
		return;
	PruneAcceptedActions();
	const FAcceptedAction* Action = AcceptedActions.Find(Packet.action_instance_id());
	if (Action == nullptr || Action->SourceObjectId != Packet.source_object_id())
	{
		// A damage packet without verified action metadata is intentionally not
		// guessed as Terra. The server sends action metadata first on this stream.
		return;
	}
	const TObjectPtr<AS1Player>* SourceEntry = Players.Find(Packet.source_object_id());
	AS1Player* Source = SourceEntry != nullptr ? SourceEntry->Get() : nullptr;
	if (!IsValid(Source))
		return;
	const US1CombatImpactSettings* Settings = GetDefault<US1CombatImpactSettings>();
	const FS1CombatImpactRule* Rule = Settings->ImpactRules.FindByPredicate(
		[Action](const FS1CombatImpactRule& Candidate)
		{
			return Candidate.ActionType == Action->ActionType;
		});
	if (Rule == nullptr)
		return;
	if (!LoadedImpactParticles.Contains(Action->ActionType))
		PreloadImpactParticles();
	const TObjectPtr<UParticleSystem>* Particle = LoadedImpactParticles.Find(Action->ActionType);
	UWorld* World = GetWorld();
	if (Particle == nullptr || *Particle == nullptr || World == nullptr)
		return;

	const FVector Location = Target->GetActorTransform().TransformPositionNoScale(
		Rule->RelativeLocation);
	FRotator Rotation = FRotator::ZeroRotator;
	if (Rule->Facing == ES1CombatImpactFacing::SourceToTarget)
	{
		FVector Direction = Target->GetActorLocation() - Source->GetActorLocation();
		Direction.Z = 0.f;
		if (!Direction.IsNearlyZero())
			Rotation = Direction.Rotation();
	}
	else if (Rule->Facing == ES1CombatImpactFacing::TargetFacing)
	{
		Rotation = Target->GetActorRotation();
	}
	Rotation += Rule->RotationOffset;
	const float Scale = FMath::Max(0.01f, Rule->UniformScale);
	UParticleSystemComponent* Effect = UGameplayStatics::SpawnEmitterAtLocation(
		World, *Particle, FTransform(Rotation, Location, FVector(Scale)),
		true, EPSCPoolMethod::None, true);
	if (!IsValid(Effect))
		return;

	RecentImpactEventSequences.Add(Packet.room_event_sequence());
	RecentImpactEventOrder.Add(Packet.room_event_sequence());
	if (RecentImpactEventOrder.Num() > 256)
	{
		RecentImpactEventSequences.Remove(RecentImpactEventOrder[0]);
		RecentImpactEventOrder.RemoveAt(0);
	}
	const TWeakObjectPtr<UParticleSystemComponent> WeakEffect(Effect);
	FTimerHandle CleanupHandle;
	World->GetTimerManager().SetTimer(CleanupHandle,
		FTimerDelegate::CreateLambda([WeakEffect]()
		{
			if (UParticleSystemComponent* Component = WeakEffect.Get())
				Component->DestroyComponent();
		}), FMath::Max(0.1f, Rule->MaxLifeSeconds), false);
	UE_LOG(LogTemp, Verbose,
		TEXT("Combat impact played. Event[%u], Action[%llu], Source[%llu], Target[%llu], Type[%d]"),
		Packet.room_event_sequence(), Packet.action_instance_id(),
		Packet.source_object_id(), Packet.target_object_id(), Action->ActionType);
}

bool US1WorldEntitySubsystem::ContainsPlayer(uint64 ObjectId) const
{
	return Players.Contains(ObjectId);
}

bool US1WorldEntitySubsystem::ContainsEntity(uint64 ObjectId) const
{
	return Players.Contains(ObjectId) || Monsters.Contains(ObjectId);
}

bool US1WorldEntitySubsystem::HasLocalPlayer() const
{
	return IsValid(MyPlayer);
}

void US1WorldEntitySubsystem::UpdateLootHints(uint64 CharacterId,const FString& Selected,bool bPending)
{
 for(const auto& Entry:LootActors)if(IsValid(Entry.Value))
  Entry.Value->ShowOwnerPickupHint(CharacterId,Entry.Key==Selected,bPending);
}
