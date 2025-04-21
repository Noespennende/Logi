// Copyright Epic Games, Inc. All Rights Reserved.

#include "LogiCommands.h"

#define LOCTEXT_NAMESPACE "FLogiModule"

void FLogiCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Logi", "Convert current project to Logi project", EUserInterfaceActionType::Button, FInputChord());
	FSlateIcon(FLogiStyle::GetStyleSetName(), "Logi.PluginAction");
	FLogiStyle::ReloadTextures();
}


#undef LOCTEXT_NAMESPACE
