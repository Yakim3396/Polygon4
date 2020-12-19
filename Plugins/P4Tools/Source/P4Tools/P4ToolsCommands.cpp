// Copyright Epic Games, Inc. All Rights Reserved.

#include "P4ToolsCommands.h"

#define LOCTEXT_NAMESPACE "FP4ToolsModule"

void FP4ToolsCommands::RegisterCommands()
{
    UI_COMMAND(OpenPluginWindow, "P4Tools", "Bring up P4Tools window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
