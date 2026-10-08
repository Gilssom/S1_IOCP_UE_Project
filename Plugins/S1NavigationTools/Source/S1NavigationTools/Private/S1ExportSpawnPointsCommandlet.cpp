#include "S1ExportSpawnPointsCommandlet.h"
#include "Game/S1MonsterSpawnPoint.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Components/SceneComponent.h"
#include "NavMesh/RecastNavMesh.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"

US1ExportSpawnPointsCommandlet::US1ExportSpawnPointsCommandlet()
{
	IsClient = false; IsServer = false; IsEditor = true;
	LogToConsole = true; ShowErrorCount = true;
}

int32 US1ExportSpawnPointsCommandlet::Main(const FString& Params)
{
	FString Map = TEXT("/Game/Maps/MainMap"), Output;
	FParse::Value(*Params, TEXT("Map="), Map);
	FParse::Value(*Params, TEXT("Output="), Output);
	if (Map != TEXT("/Game/Maps/MainMap") || Output.IsEmpty()) return 2;
	UPackage* Package = LoadPackage(nullptr, *Map, LOAD_None);
	if (Package && Package->IsDirty())
	{ UE_LOG(LogTemp, Error, TEXT("Save MainMap before exporting; export never saves a map.")); return 9; }
	UWorld* World = Package ? UWorld::FindWorldInPackage(Package) : nullptr;
	if (!World || !World->PersistentLevel) return 3;
	TArray<AS1MonsterSpawnPoint*> Markers;
	ARecastNavMesh* Nav = nullptr;
	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		if (auto* Marker = Cast<AS1MonsterSpawnPoint>(Actor)) Markers.Add(Marker);
		if (auto* Candidate = Cast<ARecastNavMesh>(Actor))
			if (Candidate->CanBeMainNavData()) Nav = Candidate;
	}
	if (Markers.IsEmpty() || Markers.Num() > 16 || !Nav)
	{ UE_LOG(LogTemp, Error, TEXT("Saved MainMap requires 1..16 spawn markers and main Recast navmesh.")); return 5; }
	Markers.Sort([](const AS1MonsterSpawnPoint& A, const AS1MonsterSpawnPoint& B) { return A.SpawnPointId < B.SpawnPointId; });
	TSet<int32> Ids;
	FString Text = FString::Printf(TEXT("S1SPAWN 2 MainMap\nPOINTS %d\n"), Markers.Num());
	for (AS1MonsterSpawnPoint* Marker : Markers)
	{
		if (Marker->SpawnPointId <= 0 || Ids.Contains(Marker->SpawnPointId))
		{ UE_LOG(LogTemp, Error, TEXT("Duplicate/nonpositive spawn ID: %d"), Marker->SpawnPointId); return 4; }
		Ids.Add(Marker->SpawnPointId);
		// Dusk remains the only registered type. Unknown IDs are not rendered as Dusk fallback.
		if (Marker->MonsterDefinitionId != 1)
		{ UE_LOG(LogTemp, Error, TEXT("Unregistered monster definition: %d"), Marker->MonsterDefinitionId); return 6; }
		Marker->GetRootComponent()->UpdateComponentToWorld();
		const FVector Feet = Marker->GetActorLocation();
		const FRotator Facing = Marker->GetActorRotation();
		if (Feet.ContainsNaN() || Facing.ContainsNaN() ||
			!Marker->GetActorScale3D().Equals(FVector::OneVector, 0.001f) ||
			FMath::Abs(Feet.Z + 0.5) > 0.01 || FMath::Abs(Facing.Pitch) > 0.01 || FMath::Abs(Facing.Roll) > 0.01 ||
			FMath::Abs(Feet.X) > 100000. || FMath::Abs(Feet.Y) > 100000.)
		{ UE_LOG(LogTemp, Error, TEXT("Spawn %d requires unit scale, feet Z=-0.5, yaw-only rotation and bounded finite coordinates."), Marker->SpawnPointId); return 6; }
		FNavLocation Projected;
		if (!Nav->ProjectPoint(Feet, Projected, FVector(2.f, 2.f, 150.f)) ||
			FVector::Dist2D(Feet, Projected.Location) > 2.f)
		{ UE_LOG(LogTemp, Error, TEXT("Spawn %d outside saved navigable space."), Marker->SpawnPointId); return 7; }
		Text += FString::Printf(TEXT("P %d %d %d %.6f %.6f -0.500000 %.6f\n"),
			Marker->SpawnPointId, Marker->MonsterDefinitionId, Marker->bEnabled ? 1 : 0,
			Feet.X, Feet.Y, FRotator::NormalizeAxis(Facing.Yaw));
		UE_LOG(LogTemp, Display, TEXT("Spawn point validated. Id[%d] Definition[%d] Feet[%s] Enabled[%d]"),
			Marker->SpawnPointId, Marker->MonsterDefinitionId, *Feet.ToString(), Marker->bEnabled);
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
	const FString Temporary = Output + TEXT(".tmp");
	if (!FFileHelper::SaveStringToFile(Text, *Temporary, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM) ||
		!IFileManager::Get().Move(*Output, *Temporary, true, true)) return 8;
	UE_LOG(LogTemp, Display, TEXT("S1 spawn export complete. Points[%d] Output[%s]"), Markers.Num(), *Output);
	return 0;
}
