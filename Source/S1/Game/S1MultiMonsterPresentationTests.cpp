#if WITH_DEV_AUTOMATION_TESTS
#include "World/S1WorldEntitySubsystem.h"
#include "Game/S1DuskMinion.h"
#include "Game/S1MonsterData.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextRenderComponent.h"
#include "Misc/AutomationTest.h"
#include "Protocol.pb.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1MultiMonsterPresentationTest,
	"S1.World.MultiMonsterPresentation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FS1MultiMonsterPresentationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	auto* Entities = NewObject<US1WorldEntitySubsystem>(World);
	auto* BlueprintClass = LoadClass<AS1DuskMinion>(nullptr,
		TEXT("/Game/S1/Monsters/BP_S1DuskMinion.BP_S1DuskMinion_C"));
	TestNotNull(TEXT("Authored Dusk class exists"), BlueprintClass);
	const auto* Visual=FS1MonsterData::FindPresentation(FS1MonsterData::LoadPresentations(),1);
	TestTrue(TEXT("Presentation table anchors the exact Dusk Blueprint"),Visual&&Visual->ActorClass.Get()==BlueprintClass);
	TestTrue(TEXT("Native hard table cook roots"),GetDefault<US1WorldEntitySubsystem>()->MonsterDefinitions.Get()==FS1MonsterData::LoadDefinitions()&&
		GetDefault<US1WorldEntitySubsystem>()->MonsterPresentations.Get()==FS1MonsterData::LoadPresentations());
	TestFalse(TEXT("Unknown definition has no Dusk fallback"),FS1MonsterData::FindPresentation(FS1MonsterData::LoadPresentations(),999999)!=nullptr);
	Protocol::ObjectInfo Info;
	Info.set_creature_type(Protocol::CREATURE_TYPE_MONSTER);
	Info.set_monster_definition_id(1); Info.set_current_hp(200); Info.set_max_hp(200);
	Info.set_monster_state(Protocol::MONSTER_STATE_IDLE);
	Info.mutable_pos_info()->set_z(61.5f);
	for (uint64 Id : {101ull, 102ull, 103ull})
	{
		Info.set_object_id(Id); Info.mutable_pos_info()->set_object_id(Id);
		Info.mutable_pos_info()->set_x(static_cast<float>(Id - 100) * 100.f);
		TestTrue(TEXT("Distinct monster spawned"), Entities->SpawnMonster(Info));
	}
	TestEqual(TEXT("Three independent registry entries"), Entities->Monsters.Num(), 3);
	for (const auto& Entry : Entities->Monsters)
		if (auto* Actor = Entry.Value.Get())
			TestTrue(TEXT("Uses authored Blueprint, not native fallback"), Actor->GetClass() == BlueprintClass);
	TestTrue(TEXT("Duplicate snapshot is idempotent"), Entities->SpawnMonster(Info));
	TestEqual(TEXT("Duplicate creates no fourth actor"), Entities->Monsters.Num(), 3);
	Protocol::S_DAMAGE_RESULT Damage;
	Damage.set_source_object_id(99); Damage.set_action_instance_id(700);
	Damage.set_damage(25); Damage.set_current_hp(175); Damage.set_max_hp(200);
	Damage.set_hit_index(0);
	for (uint64 Id : {101ull, 102ull, 103ull})
	{
		Damage.set_target_object_id(Id); Damage.set_room_event_sequence(static_cast<uint32>(Id));
		TestTrue(TEXT("Same action hits each target independently"), Entities->ApplyDamageResult(Damage));
		if (AS1DuskMinion* Actor = Entities->Monsters.FindRef(Id).Get())
			if (auto* Label = Actor->FindComponentByClass<UTextRenderComponent>())
				TestTrue(TEXT("Target displays its server HP"), Label->Text.ToString().Contains(TEXT("175")));
	}
	TestFalse(TEXT("Duplicate target/hit rejected"), Entities->ApplyDamageResult(Damage));
	Damage.set_hit_index(1); Damage.set_current_hp(137); Damage.set_damage(38); Damage.set_room_event_sequence(104);
	TestTrue(TEXT("Second Q hit on same target is accepted"), Entities->ApplyDamageResult(Damage));
	TestTrue(TEXT("Only selected object despawned"), Entities->DespawnEntity(102));
	TestFalse(TEXT("Removed object absent"), Entities->ContainsEntity(102));
	TestTrue(TEXT("Survivor retained"), Entities->ContainsEntity(101) && Entities->ContainsEntity(103));
	Info.set_object_id(202); Info.mutable_pos_info()->set_object_id(202);
	TestTrue(TEXT("Respawn uses new object identity"), Entities->SpawnMonster(Info));
	TestEqual(TEXT("Three entries after independent respawn"), Entities->Monsters.Num(), 3);
	Info.set_object_id(999); Info.set_monster_definition_id(999);
	AddExpectedMessagePlain(TEXT("Monster presentation rejected. Definition[999]"),ELogVerbosity::Warning);
	TestFalse(TEXT("Unregistered definition is not a Dusk fallback"), Entities->SpawnMonster(Info));
	Entities->ResetRegistry();
	TestEqual(TEXT("Reconnect reset clears all monster actors"), Entities->Monsters.Num(), 0);
	GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
	return true;
}
#endif
