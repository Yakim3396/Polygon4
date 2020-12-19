// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Implements the tool window.
 */
class SSampleModuleWindow : public SCompoundWidget
{
public:
    /** Default constructor. */
    SSampleModuleWindow();

    /** Virtual destructor. */
    virtual ~SSampleModuleWindow();

    SLATE_BEGIN_ARGS(SSampleModuleWindow)
    {
    }
    SLATE_END_ARGS()

    /**
     * Constructs this widget.
     */
    void Construct(const FArguments& InArgs);

private:
};
