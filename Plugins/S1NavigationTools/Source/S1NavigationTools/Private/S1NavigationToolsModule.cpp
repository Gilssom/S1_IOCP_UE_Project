#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "Framework/Commands/UIAction.h"
#include "S1ExportSpawnPointsCommandlet.h"
#include "S1ExportMonsterDefinitionsCommandlet.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"

class FS1NavigationToolsModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FS1NavigationToolsModule::RegisterMenus));
	}
	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}
private:
	void RegisterMenus()
	{
		FToolMenuOwnerScoped Owner(this);
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		Menu->FindOrAddSection("S1ServerAuthoring").AddMenuEntry(
			"S1ExportMonsterDefinitions",FText::FromString(TEXT("Export Monster Definitions")),
			FText::FromString(TEXT("Validate saved monster Data Tables and export server definitions; does not deploy.")),FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([]
			{
				const FString Output=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("ServerData/MonsterDefinitions.s1monster"));
				const int32 Result=NewObject<US1ExportMonsterDefinitionsCommandlet>()->Main(FString::Printf(TEXT("Output=\"%s\""),*Output));
				FMessageDialog::Open(EAppMsgType::Ok,FText::FromString(Result==0?TEXT("Monster ExportComplete. Validate/deploy export then restart GameServer."):TEXT("Monster export failed. Save intended tables and inspect Output Log.")));
			})));
		Menu->FindOrAddSection("S1ServerAuthoring", FText::FromString(TEXT("S1 Server Authoring"))).AddMenuEntry(
			"S1ExportDuskSpawn", FText::FromString(TEXT("Export Monster Spawn Points")),
			FText::FromString(TEXT("Export saved MainMap's flat spawn markers. Does not save or deploy to the server.")),
			FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([]
			{
				const FString Output = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("ServerData/MainMap.spawn"));
				const int32 Result = NewObject<US1ExportSpawnPointsCommandlet>()->Main(FString::Printf(TEXT("Output=\"%s\""), *Output));
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Result == 0
					? FString::Printf(TEXT("Export complete: %s\nDeploy this file and restart GameServer to apply."), *Output)
					: TEXT("Export failed. Check Output Log. Save MainMap and verify unique IDs, registered types and flat markers on saved navigation.")));
			})));
	}
};

IMPLEMENT_MODULE(FS1NavigationToolsModule, S1NavigationTools)
