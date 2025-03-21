// Copyright Epic Games, Inc. All Rights Reserved.

#include "Logi.h"
#include "LogiStyle.h"
#include "LogiCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "../Public/LogiStyle.h"
#include "../Public/Logi.h"
#include "HAL/FileManager.h" // For editing files
#include "AssetViewUtils.h" 

static const FName LogiTabName("Logi");

#define LOCTEXT_NAMESPACE "FLogiModule"

void FLogiModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLogiStyle::Initialize();
	FLogiStyle::ReloadTextures();

	FLogiCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLogiCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FLogiModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLogiModule::RegisterMenus));
}

void FLogiModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLogiStyle::Shutdown();

	FLogiCommands::Unregister();
}


void FLogiModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	bool folderCreated;
	FString statusMessage;

	CreateFolderStructure(folderCreated, statusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *statusMessage);
}


void FLogiModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLogiCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLogiCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

//Suport functions
void CreateFolder(FString FolderPath, bool& folderCreated, FString& statusMessage) {

	// Convert project path to absolute path
	FString AbsoluteFolderPath;
	if (!FPackageName::TryConvertLongPackageNameToFilename(FolderPath, AbsoluteFolderPath)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Creating folders failed - Failed to convert folder path to absolute path: '% s'"), *AbsoluteFolderPath);
		return;
	}

	// Create a new folder
	if (!IFileManager::Get().MakeDirectory(*AbsoluteFolderPath, true)) {
		folderCreated = false;
		statusMessage = FString::Printf(TEXT("Creating folders failed - Failed to create folder: '% s'"), *AbsoluteFolderPath);
		return;
	}

	//return success
	folderCreated = true;
	statusMessage = FString::Printf(TEXT("Creating folders succeeded - Folder created: '% s'"), *AbsoluteFolderPath);

}

void CreateFolderStructure(bool& folderCreated, FString& statusMessage) {
	CreateFolder("content/Logi_ThermalCamera", folderCreated, statusMessage);


	AssetViewUtils::SetPathColor("content/Logi_ThermalCamera", FColor::Blue);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogiModule, Logi)