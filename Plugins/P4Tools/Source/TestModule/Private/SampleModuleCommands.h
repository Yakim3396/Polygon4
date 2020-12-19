// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FMenuBuilder;

/**
 * Class that holds all SampleModule commands.
 */
class FSampleModuleCommands : public TCommands<FSampleModuleCommands>
{
public:
    /** Default constructor. */
    FSampleModuleCommands();

    /** Initialize commands. */
    virtual void RegisterCommands() override;

public:

};
