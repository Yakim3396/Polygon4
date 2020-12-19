// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class FMenuBuilder;
class FSpawnTabArgs;
class FTabManager;
class ITargetDeviceProxyManager;
class SButton;
class SWindow;

/**
 * Implements the main window
 */
class SP4ToolsWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SP4ToolsWindow)
    {
    }
    SLATE_END_ARGS()

public:
    /**
     * Constructs the main window
     *
     * @param InArgs The Slate argument list.
     * @param ConstructUnderMajorTab The major tab which will contain the tools.
     * @param ConstructUnderWindow The window in which this widget is being constructed.
     */
    void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

    TSharedPtr<FTabManager> GetTabManager() const
    {
        return TabManager;
    }

protected:
    /**
     * Fills the Window menu with menu items.
     *
     * @param MenuBuilder The multi-box builder that should be filled with content for this pull-down menu.
     * @param TabManager A Tab Manager from which to populate tab spawner menu items.
     */
    static void FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager);

private:
    /** Callback for spawning tabs. */
    TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const;

private:
    /** Holds the tab manager that manages the front-end's tabs. */
    TSharedPtr<FTabManager> TabManager;
};
