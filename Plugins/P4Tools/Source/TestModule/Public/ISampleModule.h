// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"

class SWidget;

/**
 * Interface for the SampleModule module.
 */
class ISampleModule : public IModuleInterface
{
public:
    /**
     * Creates the main window for the SampleModule.
     */
    virtual TSharedRef<SWidget> CreateSampleModuleWindow(const TSharedRef<SDockTab>& ConstructUnderMajorTab) = 0;
};
