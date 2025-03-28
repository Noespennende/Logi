// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"

class FToolBarBuilder;
class FMenuBuilder;

class FLogiModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	void SetupThermalSettings(UWorld* World);
	
private:

	void RegisterMenus();
	UMaterialParameterCollection* EnsureThermalSettingsExist(UWorld* World);


private:
	TSharedPtr<class FUICommandList> PluginCommands;

};
