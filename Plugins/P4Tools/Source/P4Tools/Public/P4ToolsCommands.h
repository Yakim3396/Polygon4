// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "P4ToolsStyle.h"

class FP4ToolsCommands : public TCommands<FP4ToolsCommands>
{
public:

	FP4ToolsCommands()
		: TCommands<FP4ToolsCommands>(TEXT("P4Tools"), NSLOCTEXT("Contexts", "P4Tools", "P4Tools Plugin"), NAME_None, FP4ToolsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};