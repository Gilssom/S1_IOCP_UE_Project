#include "Loot/S1LootActor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1LootActorTest,"S1.Loot.ActorLifecycle",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FS1LootActorTest::RunTest(const FString& Parameters)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("Transient world"),World)) return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 AS1LootActor* Actor=World->SpawnActor<AS1LootActor>();
 if(TestNotNull(TEXT("Loot actor"),Actor))
 {
  const FString Key=TEXT("00112233445566778899aabbccddeeff");
  const FVector Floor(10,20,-0.5);
  TestFalse(TEXT("Unconfigured no hint"),Actor->IsPickupHintVisible());
  TestFalse(TEXT("Reject malformed ID"),Actor->ApplyServerDrop(TEXT("bad"),42,1,Floor));
  TestFalse(TEXT("Reject zero ID"),Actor->ApplyServerDrop(FString::ChrN(32,TEXT('0')),42,1,Floor));
  TestTrue(TEXT("Apply server metadata"),Actor->ApplyServerDrop(Key,42,1,Floor));
  TestTrue(TEXT("Duplicate snapshot allowed"),Actor->ApplyServerDrop(Key,42,1,Floor));
  TestFalse(TEXT("Immutable ownership"),Actor->ApplyServerDrop(Key,43,1,Floor));
  TestFalse(TEXT("No client collision authority"),Actor->GetActorEnableCollision());
  TestEqual(TEXT("Server floor position, no capsule offset"),Actor->GetActorLocation(),Floor);
  Actor->ShowOwnerPickupHint(43,true,false);
  TestFalse(TEXT("Other character hint hidden"),Actor->IsPickupHintVisible());
  Actor->ShowOwnerPickupHint(42,true,false);
  TestTrue(TEXT("Owner selected hint"),Actor->IsPickupHintVisible());
  Actor->ShowOwnerPickupHint(42,true,true);
  TestTrue(TEXT("Pending preserves visible drop"),Actor->IsPickupHintVisible());
  TestFalse(TEXT("Pending does not retire/drop item"),Actor->IsRetired());
  Actor->ShowOwnerPickupHint(42,false,false);
  TestFalse(TEXT("Only selected candidate hint"),Actor->IsPickupHintVisible());
  Actor->RetirePresentation();Actor->RetirePresentation();
  TestTrue(TEXT("Retired once and hidden"),Actor->IsRetired()&&Actor->IsHidden());
  TestTrue(TEXT("Drop metadata cleared"),Actor->GetDropKey().IsEmpty());
  TestFalse(TEXT("Late packet cannot resurrect retired actor"),Actor->ApplyServerDrop(Key,42,1,Floor));
  Actor->ShowOwnerPickupHint(42,true,false);
  TestFalse(TEXT("No stale hint after cleanup"),Actor->IsPickupHintVisible());
  Actor->Destroy();
 }
 World->DestroyWorld(false);GEngine->DestroyWorldContext(World);
 // Synthetic ownership/lifecycle checks only. No rendered GPU effect/network grant claim.
 return !HasAnyErrors();
}
#endif
