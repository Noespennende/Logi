// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LogiStyle.h"

class FLogiCommands : public TCommands<FLogiCommands>
{
public:

	FLogiCommands()
		: TCommands<FLogiCommands>(TEXT("Logi"), NSLOCTEXT("Contexts", "Logi", "Logi Plugin"), NAME_None, FLogiStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
