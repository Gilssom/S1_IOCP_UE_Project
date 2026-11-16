#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Game/S1MyPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FS1PortalCameraTest,"S1.Level.PortalCameraPlacement",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FS1PortalCameraTest::RunTest(const FString&)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 if(!TestNotNull(TEXT("Test world"),World))return false;
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 AS1MyPlayer* Player=World->SpawnActor<AS1MyPlayer>();
 if(!TestNotNull(TEXT("Player"),Player)){GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return false;}
 USpringArmComponent* Boom=Player->GetCameraBoom();
 Boom->TargetOffset=FVector(0,0,34);
 Boom->PreviousDesiredLoc=FVector::ZeroVector;Boom->PreviousArmOrigin=FVector::ZeroVector;
 Player->SetActorLocation(FVector(4490,2480,196),false,nullptr,ETeleportType::TeleportPhysics);
 Player->SnapCameraAfterServerPlacement();
 const FVector Expected=Boom->GetComponentLocation()+Boom->TargetOffset;
 TestTrue(TEXT("Camera lag target snaps to placed player"),Boom->PreviousDesiredLoc.Equals(Expected,0.01));
 TestTrue(TEXT("Previous arm origin snaps to placed player"),Boom->PreviousArmOrigin.Equals(Expected,0.01));
 TestTrue(TEXT("Camera keeps configured lag"),Boom->bEnableCameraLag);
 GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return true;
}
#endif
