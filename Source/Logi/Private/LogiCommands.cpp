// Copyright Epic Games, Inc. All Rights Reserved.

#include "LogiCommands.h"

#define LOCTEXT_NAMESPACE "FLogiModule"

void FLogiCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Logi", "Execute Logi action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
