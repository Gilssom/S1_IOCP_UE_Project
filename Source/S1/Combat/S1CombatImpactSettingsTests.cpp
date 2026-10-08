#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/S1CombatImpactSettings.h"

#include "Misc/AutomationTest.h"
#include "Particles/ParticleSystem.h"
#include "Protocol.pb.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1CombatImpactSettingsTest,
	"S1.Combat.ImpactSettings", EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::EngineFilter)

bool FS1CombatImpactSettingsTest::RunTest(const FString& Parameters)
{
	const US1CombatImpactSettings* Settings = GetDefault<US1CombatImpactSettings>();
	if (!TestNotNull(TEXT("Impact settings"), Settings))
		return false;
	if (!TestEqual(TEXT("Configured rule count"), Settings->ImpactRules.Num(), 2))
		return false;
	const FS1CombatImpactRule& Rule = Settings->ImpactRules[0];
	TestEqual(TEXT("Action type"), Rule.ActionType,
		static_cast<int32>(Protocol::COMBAT_ACTION_KNIGHT_PRIMARY));
	TestEqual(TEXT("Particle path"), Rule.ParticleAsset.ToString(),
		FString(TEXT("/Game/ParagonTerra/FX/Particles/Terra/Abilities/Primary/FX/"
			"P_Terra_Primary_Impact.P_Terra_Primary_Impact")));
	TestFalse(TEXT("Particle path populated"), Rule.ParticleAsset.IsNull());
	TestNotNull(TEXT("Particle loads"),
		Cast<UParticleSystem>(Rule.ParticleAsset.TryLoad()));
	TestEqual(TEXT("Torso offset Z"), Rule.RelativeLocation.Z, 30.0);
	TestEqual(TEXT("Uniform scale"), Rule.UniformScale, 0.7f);
	TestEqual(TEXT("Maximum lifetime"), Rule.MaxLifeSeconds, 2.5f);
	const FS1CombatImpactRule& QRule = Settings->ImpactRules[1];
	TestEqual(TEXT("Explicit Q action mapping"), QRule.ActionType,
		static_cast<int32>(Protocol::COMBAT_ACTION_KNIGHT_Q));
	TestEqual(TEXT("Q deliberately reuses the approved impact"),
		QRule.ParticleAsset.ToString(), Rule.ParticleAsset.ToString());
	return !HasAnyErrors();
}

#endif
