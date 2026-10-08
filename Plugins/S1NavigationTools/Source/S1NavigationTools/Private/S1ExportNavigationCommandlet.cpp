#include "S1ExportNavigationCommandlet.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "NavMesh/RecastNavMesh.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	constexpr int32 NavigationFormatVersion = 2;

	struct FExportEdge
	{
		int32 From = INDEX_NONE;
		int32 To = INDEX_NONE;
		FVector Left = FVector::ZeroVector;
		FVector Right = FVector::ZeroVector;
	};

	// Saved static collision-blocking geometry only. Render bounds are deliberately
	// conservative OBBs; concave mesh openings can be overblocked, never inferred clear.
	bool ExportLootObstacles(UWorld* World, const FString& Path)
	{
		TArray<FString> Rows;
		TArray<ULevel*> Levels = World->GetLevels();
		Levels.AddUnique(World->PersistentLevel); // Loaded but uninitialized commandlet world.
		for (ULevel* Level : Levels)
		for (AActor* Actor : Level->Actors)
		{
			if (!Actor) continue;
			TInlineComponentArray<UPrimitiveComponent*> Components(Actor);
			UE_LOG(LogTemp, Display, TEXT("Pickup export actor %s Components[%d]"), *Actor->GetName(), Components.Num());
			for (UPrimitiveComponent* Component : Components)
			{
				UE_LOG(LogTemp, Display, TEXT("Pickup export component %s Mobility[%d] Query[%d] PawnResponse[%d]"),
					*Component->GetName(), static_cast<int32>(Component->Mobility), Component->IsQueryCollisionEnabled(), static_cast<int32>(Component->GetCollisionResponseToChannel(ECC_Pawn)));
				if (!Component->IsQueryCollisionEnabled() ||
					Component->GetCollisionResponseToChannel(ECC_Pawn) != ECR_Block) continue;
				auto* Mesh = Cast<UStaticMeshComponent>(Component);
				if (!Mesh || !Mesh->GetStaticMesh() || Mesh->IsA<UInstancedStaticMeshComponent>())
				{
					UE_LOG(LogTemp, Error, TEXT("Unsupported static pickup blocker: %s"), *Component->GetPathName());
					return false;
				}
				// Current saved floor/test wall are native, non-ticking geometry but
				// Mobility=Movable. Capture this immutable layout; do not export moving AI/BPs.
				if (Mesh->IsSimulatingPhysics() || Actor->PrimaryActorTick.bCanEverTick ||
					Actor->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
				{UE_LOG(LogTemp, Error, TEXT("Dynamic pickup blocker unsupported: %s"), *Component->GetPathName());return false;}
				FBox Bounds = Mesh->GetStaticMesh()->GetBoundingBox();
				if (UBodySetup* Body = Mesh->GetBodySetup())
				{
					const FBox Collision = Body->AggGeom.CalcAABB(FTransform::Identity);
					if (Collision.IsValid) Bounds += Collision; // Include custom collision outside render bounds.
				}
				// LoadPackage worlds have unregistered components; cached world transforms
				// are not yet current. Update this transient loaded hierarchy, never save it.
				Mesh->UpdateComponentToWorld();
				const FTransform Transform = Mesh->GetComponentTransform();
				const FVector Center = Transform.TransformPosition(Bounds.GetCenter());
				const FVector Extent = Bounds.GetExtent() * Transform.GetScale3D().GetAbs();
				if (!Bounds.IsValid || Center.ContainsNaN() || Extent.ContainsNaN() || Extent.GetMin() <= 0.)
				{UE_LOG(LogTemp, Error, TEXT("Invalid pickup blocker bounds: %s"), *Component->GetPathName());return false;}
				const FVector X = Transform.GetRotation().GetAxisX(), Y = Transform.GetRotation().GetAxisY(), Z = Transform.GetRotation().GetAxisZ();
				Rows.Add(FString::Printf(TEXT("B %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n"),
					Center.X,Center.Y,Center.Z,Extent.X,Extent.Y,Extent.Z,X.X,X.Y,X.Z,Y.X,Y.Y,Y.Z,Z.X,Z.Y,Z.Z));
				UE_LOG(LogTemp, Display, TEXT("Pickup blocker %s Center[%s] Extent[%s]"), *Component->GetPathName(), *Center.ToString(), *Extent.ToString());
			}
		}
		if (Rows.IsEmpty() || Rows.Num() > 4096)
		{UE_LOG(LogTemp, Error, TEXT("Invalid pickup blocker count: %d"), Rows.Num());return false;}
		Rows.Sort();
		FString Text = FString::Printf(TEXT("S1OBB 1 MainMap\nBOXES %d\n"), Rows.Num());
		for (const FString& Row : Rows) Text += Row;
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
		return FFileHelper::SaveStringToFile(Text, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

US1ExportNavigationCommandlet::US1ExportNavigationCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

int32 US1ExportNavigationCommandlet::Main(const FString& Params)
{
#if !WITH_RECAST
	UE_LOG(LogTemp, Error, TEXT("S1 navigation export requires WITH_RECAST."));
	return 1;
#else
	FString MapPath = TEXT("/Game/Maps/MainMap");
	FString OutputPath;
	FString ObstaclesPath;
	FParse::Value(*Params, TEXT("Obstacles="), ObstaclesPath);
	FParse::Value(*Params, TEXT("Map="), MapPath);
	FParse::Value(*Params, TEXT("Output="), OutputPath);
	if (OutputPath.IsEmpty() && ObstaclesPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Missing required Output= path."));
		return 2;
	}

	UPackage* Package = LoadPackage(nullptr, *MapPath, LOAD_None);
	if (Package == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load map package: %s"), *MapPath);
		return 3;
	}

	UWorld* World = UWorld::FindWorldInPackage(Package);
	if (World == nullptr || World->PersistentLevel == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Map package has no persistent world: %s"), *MapPath);
		return 4;
	}
	if (!ObstaclesPath.IsEmpty())
	{
		if (MapPath != TEXT("/Game/Maps/MainMap") || !ExportLootObstacles(World, ObstaclesPath)) return 8;
		UE_LOG(LogTemp, Display, TEXT("S1 pickup obstacles export complete: %s"), *ObstaclesPath);
		if (OutputPath.IsEmpty()) return 0; // Leave existing navigation data unchanged.
	}

	ARecastNavMesh* NavMesh = nullptr;
	for (AActor* Actor : World->PersistentLevel->Actors)
	{
		if (ARecastNavMesh* Candidate = Cast<ARecastNavMesh>(Actor))
		{
			if (Candidate->CanBeMainNavData())
			{
				NavMesh = Candidate;
				break;
			}
		}
	}
	if (NavMesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No main RecastNavMesh found in %s"), *MapPath);
		return 5;
	}

	TArray<FNavPoly> Polys;
	TArray<FNavTileRef> TileRefs;
	NavMesh->GetAllNavMeshTiles(TileRefs);
	for (const FNavTileRef TileRef : TileRefs)
	{
		TArray<FNavPoly> TilePolys;
		if (NavMesh->GetPolysInTile(TileRef, TilePolys))
		{
			Polys.Append(MoveTemp(TilePolys));
		}
	}
	if (Polys.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("RecastNavMesh contains no polygons."));
		return 6;
	}

	TMap<NavNodeRef, int32> NodeIds;
	NodeIds.Reserve(Polys.Num());
	for (int32 Index = 0; Index < Polys.Num(); ++Index)
	{
		NodeIds.Add(Polys[Index].Ref, Index);
	}

	TArray<FExportEdge> Edges;
	for (int32 Index = 0; Index < Polys.Num(); ++Index)
	{
		TArray<FNavigationPortalEdge> Neighbors;
		if (!NavMesh->GetPolyNeighbors(Polys[Index].Ref, Neighbors))
		{
			continue;
		}
		for (const FNavigationPortalEdge& Neighbor : Neighbors)
		{
			const int32* To = NodeIds.Find(Neighbor.ToRef);
			if (To == nullptr || *To == Index)
			{
				continue;
			}
			FExportEdge& Edge = Edges.AddDefaulted_GetRef();
			Edge.From = Index;
			Edge.To = *To;
			Edge.Left = Neighbor.Left;
			Edge.Right = Neighbor.Right;
		}
	}

	FString Text;
	Text.Reserve((Polys.Num() * 80) + (Edges.Num() * 64));
	Text += FString::Printf(TEXT("S1NAV %d MainMap\n"), NavigationFormatVersion);
	Text += FString::Printf(TEXT("AGENT %.3f %.3f %.3f\n"),
		NavMesh->AgentRadius, NavMesh->AgentHeight, NavMesh->AgentMaxSlope);
	Text += FString::Printf(TEXT("NODES %d\n"), Polys.Num());
	for (int32 Index = 0; Index < Polys.Num(); ++Index)
	{
		const FVector& Center = Polys[Index].Center;
		Text += FString::Printf(TEXT("N %d %.3f %.3f %.3f\n"),
			Index, Center.X, Center.Y, Center.Z);
	}
	Text += FString::Printf(TEXT("EDGES %d\n"), Edges.Num());
	for (const FExportEdge& Edge : Edges)
	{
		Text += FString::Printf(TEXT("E %d %d %.3f %.3f %.3f %.3f %.3f %.3f\n"),
			Edge.From, Edge.To,
			Edge.Left.X, Edge.Left.Y, Edge.Left.Z,
			Edge.Right.X, Edge.Right.Y, Edge.Right.Z);
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(OutputPath), true);
	if (!FFileHelper::SaveStringToFile(Text, *OutputPath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write navigation export: %s"), *OutputPath);
		return 7;
	}

	UE_LOG(LogTemp, Display,
		TEXT("S1 navigation export complete. Map[%s] Nodes[%d] Edges[%d] Output[%s]"),
		*MapPath, Polys.Num(), Edges.Num(), *OutputPath);
	return 0;
#endif
}
