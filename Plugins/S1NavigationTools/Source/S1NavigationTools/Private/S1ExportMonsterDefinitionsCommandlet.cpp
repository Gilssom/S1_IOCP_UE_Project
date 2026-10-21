#include "S1ExportMonsterDefinitionsCommandlet.h"
#include "Game/S1MonsterData.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"
US1ExportMonsterDefinitionsCommandlet::US1ExportMonsterDefinitionsCommandlet()
{IsClient=false;IsServer=false;IsEditor=true;LogToConsole=true;ShowErrorCount=true;}
int32 US1ExportMonsterDefinitionsCommandlet::Main(const FString& Params)
{
 FString Output;FParse::Value(*Params,TEXT("Output="),Output);
 if(Output.IsEmpty())return 2;
 auto* D=FS1MonsterData::LoadDefinitions();auto* P=FS1MonsterData::LoadPresentations();
 if(!D||!P)return 3;
 if(D->GetOutermost()->IsDirty()||P->GetOutermost()->IsDirty())
 {UE_LOG(LogTemp,Error,TEXT("Save only the intended monster tables before Export. Export never saves assets."));return 4;}
 FString Text,Error;
 if(!FS1MonsterData::ExportText(D,P,Text,Error)){UE_LOG(LogTemp,Error,TEXT("Monster definitions rejected: %s"),*Error);return 5;}
 IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output),true);
 if(!FFileHelper::SaveStringToFile(Text,*(Output+TEXT(".tmp")),FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)||
  !IFileManager::Get().Move(*Output,*(Output+TEXT(".tmp")),true,true))return 6;
 UE_LOG(LogTemp,Display,TEXT("Monster ExportComplete: %s. Deploy validated file then restart GameServer."),*Output);return 0;
}
