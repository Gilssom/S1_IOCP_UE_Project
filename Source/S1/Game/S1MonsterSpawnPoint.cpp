#include "Game/S1MonsterSpawnPoint.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"

AS1MonsterSpawnPoint::AS1MonsterSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	bIsEditorOnlyActor = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FeetOrigin"));
#if WITH_EDITORONLY_DATA
	auto* Arrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Facing"));
	if (Arrow)
	{
		Arrow->SetupAttachment(RootComponent);
		Arrow->ArrowColor = FColor::Orange;
		Arrow->ArrowSize = 2.f;
		Arrow->SetRelativeLocation(FVector(0.f, 0.f, 62.f));
		Arrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Arrow->SetCanEverAffectNavigation(false);
	}
#endif
}
