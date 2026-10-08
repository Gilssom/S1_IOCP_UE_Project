#if WITH_DEV_AUTOMATION_TESTS

#include "Game/S1DuskMinion.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1DuskDeathPresentationTest,
	"S1.Presentation.DuskDeathFade", EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::EngineFilter)

bool FS1DuskDeathPresentationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Transient world"), World)) return false;
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto MakeDusk = [&]() { return World->SpawnActor<AS1DuskMinion>(
		AS1DuskMinion::StaticClass(), FVector(0.f, 0.f, 61.5f), FRotator::ZeroRotator, Spawn); };
	AS1DuskMinion* Dusk = MakeDusk();
	AS1DuskMinion* Living = MakeDusk();
	if (TestNotNull(TEXT("Dusk"), Dusk) && TestNotNull(TEXT("Other living Dusk"), Living))
	{
		TestNotNull(TEXT("Saved S1 death material loads"), Dusk->DeathFadeMaterial.Get());
		if (Dusk->DeathFadeMaterial != nullptr)
			TestEqual(TEXT("Death material uses existing opacity output"),
				Dusk->DeathFadeMaterial->GetBlendMode(), BLEND_Translucent);
		UMaterialInterface* Original = Dusk->GetMesh()->GetMaterial(0);
		Dusk->ApplyInitialServerState(0, 100, 100);
		Dusk->ApplyServerHealth(0, 100);
		TestFalse(TEXT("HP alone does not authorize death"), Dusk->bDeathPresentationStarted);
		Dusk->ApplyServerState(5, 0, 100);
		TestEqual(TEXT("Death disables collision immediately"),
			Dusk->GetCapsuleComponent()->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		Dusk->Tick(1.4f);
		TestEqual(TEXT("Original material retained before delay"), Dusk->GetMesh()->GetMaterial(0), Original);
		Dusk->ApplyServerState(5, 0, 100);
		TestEqual(TEXT("Duplicate DEAD does not reset elapsed"), Dusk->DeathPresentationElapsed, 1.4f);
		Dusk->Tick(0.7f);
		if (TestNotNull(TEXT("Per-actor death MID"), Dusk->DeathFadeMID.Get()))
		{
			TestTrue(TEXT("Halfway scalar is 0.25"), FMath::IsNearlyEqual(
				Dusk->DeathFadeMID->K2_GetScalarParameterValue(TEXT("FadeOut")), 0.25f, 0.0001f));
		}
		TestEqual(TEXT("Living Dusk material untouched"), Living->GetMesh()->GetMaterial(0), Original);
		Dusk->ApplyServerState(2, 100, 100);
		TestEqual(TEXT("Stale alive update cannot undo death"), Dusk->VisualState, 5);
		Dusk->Tick(0.61f); // First simulated frame just after 2.7s; avoid float boundary ambiguity.
		TestTrue(TEXT("Hidden on first frame after 2.7 seconds"), Dusk->IsHidden());
		TestTrue(TEXT("Completed once"), Dusk->bDeathPresentationComplete);
		TestTrue(TEXT("Not destroyed locally before authoritative despawn"), IsValid(Dusk));
		Living->ApplyServerState(5, 0, 100);
		Living->Tick(1.6f);
		TestTrue(TEXT("Independent MID for each actor"), Living->DeathFadeMID != Dusk->DeathFadeMID);
		Living->EndPlay(EEndPlayReason::Destroyed);
		TestNull(TEXT("Interrupted fade clears actor MID reference"), Living->DeathFadeMID.Get());
		AS1DuskMinion* Snapshot = MakeDusk();
		Snapshot->ApplyInitialServerState(5, 0, 100);
		TestTrue(TEXT("Initial DEAD hidden without inferred age"), Snapshot->IsHidden());
		TestNull(TEXT("Snapshot does not create fade MID"), Snapshot->DeathFadeMID.Get());
		Snapshot->ApplyServerState(5, 0, 100);
		Snapshot->Tick(0.1f);
		TestTrue(TEXT("Repeated snapshot DEAD stays complete"), Snapshot->bDeathPresentationComplete);
		AS1DuskMinion* Respawn = MakeDusk();
		Respawn->ApplyInitialServerState(0, 100, 100);
		TestFalse(TEXT("Fresh respawn visible"), Respawn->IsHidden());
		TestFalse(TEXT("Fresh respawn has no death lifecycle"), Respawn->bDeathPresentationStarted);
		TestEqual(TEXT("Respawn original appearance"), Respawn->GetMesh()->GetMaterial(0), Original);
		Snapshot->Destroy();
		Respawn->Destroy();
		Dusk->Destroy();
		Living->Destroy();
	}
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return !HasAnyErrors();
}

#endif
