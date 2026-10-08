#include "Loot/S1PotionPresentationSettings.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PotionSettingsTest,"S1.Loot.PresentationSettings",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FS1PotionSettingsTest::RunTest(const FString& Parameters)
{
 const auto* Settings=GetDefault<US1PotionPresentationSettings>();
 TestEqual(TEXT("Exact mesh"),Settings->WorldMesh.ToString(),
  FString(TEXT("/Game/LootSet/Meshes/Potions/potion_large02.potion_large02")));
 TestEqual(TEXT("Exact icon"),Settings->Icon.ToString(),
  FString(TEXT("/Game/_Resources/Rair_Potion.Rair_Potion")));
 TestEqual(TEXT("Exact Niagara"),Settings->DropEffect.ToString(),
  FString(TEXT("/Game/DrapEffet/VFX/NE_drop_effects01.NE_drop_effects01")));
 TestTrue(TEXT("Positive visual scale"),FMath::IsFinite(Settings->MeshScale)&&Settings->MeshScale>0.f);
 // No synchronous asset load here: an unsaved user import is not a gameplay test.
 return true;
}
#endif
