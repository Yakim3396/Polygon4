// Copyright Epic Games, Inc. All Rights Reserved.

#include "SampleModuleCommands.h"

#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "FSampleModuleCommands"

/*-----------------------------------------------------------------------------
        FSampleModuleCommands
-----------------------------------------------------------------------------*/

FSampleModuleCommands::FSampleModuleCommands()
    : TCommands<FSampleModuleCommands>(TEXT("SampleModuleCommand"),                                          // Context name for fast lookup
                                       NSLOCTEXT("Contexts", "SampleModuleCommand", "SampleModule Command"), // Localized context name for displaying
                                       NAME_None,                                                            // Parent
                                       FEditorStyle::GetStyleSetName()                                       // Icon Style Set
      )
{
}

/** UI_COMMAND takes long for the compile to optimize */
PRAGMA_DISABLE_OPTIMIZATION
void FSampleModuleCommands::RegisterCommands()
{
}
PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
