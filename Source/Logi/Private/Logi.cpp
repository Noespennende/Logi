// Copyright Epic Games, Inc. All Rights Reserved.

#include "Logi.h"
#include "LogiStyle.h"
#include "LogiCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "HAL/FileManager.h" // For editing files
#include "UObject/UnrealType.h"    
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include <functional>
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpression.h"
#include "MaterialShared.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialFunctionInterface.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Components/PrimitiveComponent.h"

#include "ThermalMaterialFunction.h"
#include "LogiOutliner.h"
#include "ThermalCamera.h"
#include "ActorPatcher.h"
#include "FolderStructureHandler.h"
#include "ThermalController.h"
#include "ThermalSettings.h"


static const FName LogiTabName("Logi");

#define LOCTEXT_NAMESPACE "FLogiModule"


//Plugin functions

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

	//Show confirmation dialogue box when the plugin button is clicked
	EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::FromString(TEXT("The Logi plugin will alter you current prosject. New actors and a post process volume will be added to your scene and all actors inn your project will have additional variables and node setups created inside their blueprints. Are you sure you want to run the Logi plugin setup?"))
	);

	//Cancel plugin if user clicks no
	if (Result != EAppReturnType::Yes)
	{
		UE_LOG(LogTemp, Warning, TEXT("Logi plugin setup was cancelled by user."));
		return;
	}

	bool bSuccess;
	FString StatusMessage;

	// Create folder structure
	Logi::FolderStructureHandler::CreateFolderStructure(bSuccess, StatusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World) {
		UE_LOG(LogTemp, Error, TEXT("Failed to get UWorld! Aborting Logi setup."));
		return;
	}

	// Create ThermalSettings
	Logi::MPCThermalSettings::SetupThermalSettings(World, bSuccess, StatusMessage);

	//Log status - ThermalSettings
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	//Create ThermalController blueprint
	Logi::ThermalController::CreateThermalController(bSuccess, StatusMessage);

	//Log status - ThermalController
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	// Create ThermalMaterialFunction Material
	Logi::ThermalMaterialFunction::CreateMaterialFunction(bSuccess, StatusMessage);

	//Log status - ThermalMaterialFunction
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	// Create ThermalCamera
	Logi::ThermalCamera::CreateThermalCamera(bSuccess, StatusMessage);

	// Log status - ThermalCamera
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	// Create ThermalMaterial
	Logi::ActorPatcher::CreateThermalMaterial(bSuccess, StatusMessage);

	// Log status - ThermalMaterial
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

	//Make all project actors logi compatible
	Logi::ActorPatcher::MakeProjectBPActorsLogiCompatible();

	Logi::LogiOutliner::AddLogiLogicToOutliner(World, bSuccess, StatusMessage);

	//Log status
	UE_LOG(LogTemp, Warning, TEXT("%s"), *StatusMessage);

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


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLogiModule, Logi)