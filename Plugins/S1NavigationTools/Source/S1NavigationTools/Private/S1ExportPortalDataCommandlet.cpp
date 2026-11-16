#include "S1ExportPortalDataCommandlet.h"
#include "Game/S1Portal.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

US1ExportPortalDataCommandlet::US1ExportPortalDataCommandlet()
{
	IsClient = false; IsServer = false; IsEditor = true;
	LogToConsole = true; ShowErrorCount = true;
}

int32 US1ExportPortalDataCommandlet::Main(const FString& Params)
{
	FString Output;
	double VillageX, VillageY, VillageZ, MainX, MainY, MainZ;
	if (!FParse::Value(*Params, TEXT("Output="), Output) || Output.IsEmpty() ||
		!FParse::Value(*Params, TEXT("VillageX="), VillageX) ||
		!FParse::Value(*Params, TEXT("VillageY="), VillageY) ||
		!FParse::Value(*Params, TEXT("VillageZ="), VillageZ) ||
		!FParse::Value(*Params, TEXT("MainX="), MainX) ||
		!FParse::Value(*Params, TEXT("MainY="), MainY) ||
		!FParse::Value(*Params, TEXT("MainZ="), MainZ))
	{
		UE_LOG(LogTemp, Error, TEXT("Output and both explicit arrival coordinates are required."));
		return 2;
	}
	const FVector Arrivals[] = { FVector(MainX, MainY, MainZ), FVector(VillageX, VillageY, VillageZ) };
	for (const FVector& Arrival : Arrivals)
	{
		if (Arrival.ContainsNaN() || Arrival.GetAbsMax() > 10000000.0)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid arrival coordinate."));
			return 2;
		}
	}
	const TCHAR* Maps[] = { TEXT("/Game/Maps/VillageMap"), TEXT("/Game/Maps/MainMap") };
	const int32 PortalIds[] = { 1, 2 };
	const int32 SourceRegions[] = { 2, 1 };
	FString Text = TEXT("S1PORTAL 1\nPORTALS 2\n");
	for (int32 Index = 0; Index < 2; ++Index)
	{
		UPackage* Package = LoadPackage(nullptr, Maps[Index], LOAD_None);
		if (!Package || Package->IsDirty())
		{
			UE_LOG(LogTemp, Error, TEXT("Saved, clean map required: %s"), Maps[Index]);
			return 3;
		}
		UWorld* World = UWorld::FindWorldInPackage(Package);
		if (!World || !World->PersistentLevel) return 3;
		AS1Portal* Portal = nullptr;
		for (AActor* Actor : World->PersistentLevel->Actors)
		{
			if (AS1Portal* Candidate = Cast<AS1Portal>(Actor))
			{
				if (Portal || Candidate->PortalId != PortalIds[Index])
				{
					UE_LOG(LogTemp, Error, TEXT("Expected exactly one portal with ID %d in %s."), PortalIds[Index], Maps[Index]);
					return 4;
				}
				Portal = Candidate;
			}
		}
		if (!Portal || !Portal->PortalRoot || !Portal->TriggerVolume) return 4;
		Portal->PortalRoot->UpdateComponentToWorld();
		Portal->TriggerVolume->UpdateComponentToWorld();
		const FTransform Transform = Portal->TriggerVolume->GetComponentTransform();
		const FVector Center = Transform.GetLocation();
		const FVector Scale = Transform.GetScale3D();
		const FVector Extent = Portal->TriggerVolume->GetUnscaledBoxExtent() * Scale.GetAbs();
		const FQuat Rotation = Transform.GetRotation();
		const FVector X = Rotation.GetAxisX(), Y = Rotation.GetAxisY(), Z = Rotation.GetAxisZ();
		if (Center.ContainsNaN() || Extent.ContainsNaN() || Rotation.ContainsNaN() ||
			Scale.GetMin() <= 0.0 || Extent.GetMin() < 1.0 || Extent.GetMax() > 5000.0)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid portal transform in %s."), Maps[Index]);
			return 5;
		}
		const FVector& Arrival = Arrivals[Index];
		Text += FString::Printf(TEXT("P %d %d %d %d %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f 0\n"),
			PortalIds[Index], SourceRegions[Index], SourceRegions[1-Index], PortalIds[1-Index],
			Center.X, Center.Y, Center.Z, Extent.X, Extent.Y, Extent.Z,
			X.X, X.Y, X.Z, Y.X, Y.Y, Y.Z, Z.X, Z.Y, Z.Z, Arrival.X, Arrival.Y, Arrival.Z);
	}
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
	const FString Temporary = Output + TEXT(".tmp");
	if (!FFileHelper::SaveStringToFile(Text, *Temporary, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM) ||
		!IFileManager::Get().Move(*Output, *Temporary, true, true)) return 6;
	UE_LOG(LogTemp, Display, TEXT("Portal data exported for offline validation only: %s"), *Output);
	return 0;
}
