#if WITH_DEV_AUTOMATION_TESTS
#include "Game/S1MonsterData.h"
#include "Game/S1DuskMinion.h"
#include "Misc/AutomationTest.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1MonsterDataTest,"S1.Monster.DataTables",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1MonsterDataTest::RunTest(const FString&)
{
 auto* D=NewObject<UDataTable>();D->RowStruct=FS1MonsterDefinitionRow::StaticStruct();
 auto* P=NewObject<UDataTable>();P->RowStruct=FS1MonsterPresentationRow::StaticStruct();
 FS1MonsterDefinitionRow Row;FS1MonsterPresentationRow Visual;Visual.DisplayName=FText::FromString(TEXT("Dusk"));Visual.ActorClass=AS1DuskMinion::StaticClass();
 D->AddRow(TEXT("Dusk"),Row);P->AddRow(TEXT("Dusk"),Visual);FString Error,Text;
 TestTrue(TEXT("Baseline valid"),FS1MonsterData::Validate(D,P,Error));
 TestTrue(TEXT("Deterministic export"),FS1MonsterData::ExportText(D,P,Text,Error));
 TestTrue(TEXT("Authoritative unchanged values"),Text.Contains(TEXT("M 1 1 1 200 28.000000 62.000000 220.000000"))&&Text.EndsWith(TEXT("10 2000 1000 420 3000 8000 4000\n")));
 Row.HitDelayMs=1000;TestFalse(TEXT("Hit must precede end"),FS1MonsterData::ValidateRow(Row,Error));Row.HitDelayMs=420;
 Row.BehaviorId=2;TestFalse(TEXT("New AI cannot be invented by row"),FS1MonsterData::ValidateRow(Row,Error));Row.BehaviorId=1;
 Row.CapsuleRadius=80;TestFalse(TEXT("Invalid capsule rejected"),FS1MonsterData::ValidateRow(Row,Error));Row.CapsuleRadius=28;
 Row.HomeRecoveryMs=0;TestFalse(TEXT("Zero recovery rejected"),FS1MonsterData::ValidateRow(Row,Error));Row.HomeRecoveryMs=4000;
 D->AddRow(TEXT("Duplicate"),Row);P->AddRow(TEXT("Duplicate"),Visual);TestFalse(TEXT("Duplicate IDs rejected"),FS1MonsterData::Validate(D,P,Error));
 D->RemoveRow(TEXT("Duplicate"));P->RemoveRow(TEXT("Duplicate"));Visual.ActorClass=nullptr;P->AddRow(TEXT("Dusk"),Visual);
 TestFalse(TEXT("Missing BP rejected"),FS1MonsterData::Validate(D,P,Error));
 auto* SavedD=FS1MonsterData::LoadDefinitions();auto* SavedP=FS1MonsterData::LoadPresentations();
 TestTrue(TEXT("Actual saved tables valid"),FS1MonsterData::Validate(SavedD,SavedP,Error));
 const auto* Actual=FS1MonsterData::FindDefinition(SavedD,1);
 TestTrue(TEXT("Saved Dusk baseline"),Actual&&Actual->MaxHP==200&&Actual->CapsuleRadius==28&&Actual->CapsuleHalfHeight==62&&Actual->HomeRecoveryMs==4000);
 TestNull(TEXT("Unknown ID rejected"),FS1MonsterData::FindDefinition(SavedD,999999));
 return true;
}
#endif
