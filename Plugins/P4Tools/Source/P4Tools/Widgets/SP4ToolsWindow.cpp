// Copyright Epic Games, Inc. All Rights Reserved.

#include "SP4ToolsWindow.h"

#include "EditorStyleSet.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Docking/WorkspaceItem.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISampleModule.h"
#include "Modules/ModuleManager.h"
#include "Textures/SlateIcon.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SP4ToolsWindow"

/* Local constants
 *****************************************************************************/

static const FName SampleModuleTabId0("SampleModule0");
static const FName SampleModuleTabId1("SampleModule1");
static const FName SampleModuleTabId2("SampleModule2");
static const FName SampleModuleTabId3("SampleModule3");

/* SP4ToolsWindow interface
 *****************************************************************************/

void SP4ToolsWindow::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
    // create & initialize tab manager
    TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);
    TSharedRef<FWorkspaceItem> AppMenuGroup = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("SampleModuleMenuGroupName", "SampleModule"));

    TabManager->RegisterTabSpawner(SampleModuleTabId0, FOnSpawnTab::CreateRaw(this, &SP4ToolsWindow::HandleTabManagerSpawnTab, SampleModuleTabId0))
        .SetDisplayName(LOCTEXT("SampleModuleTitle0", "SampleModule0"))
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "SessionFrontEnd.Tabs.Tools"))
        .SetGroup(AppMenuGroup);

    TabManager->RegisterTabSpawner(SampleModuleTabId1, FOnSpawnTab::CreateRaw(this, &SP4ToolsWindow::HandleTabManagerSpawnTab, SampleModuleTabId1))
        .SetDisplayName(LOCTEXT("SampleModuleTitle1", "SampleModule1"))
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "SessionFrontEnd.Tabs.Tools"))
        .SetGroup(AppMenuGroup);

    TabManager->RegisterTabSpawner(SampleModuleTabId2, FOnSpawnTab::CreateRaw(this, &SP4ToolsWindow::HandleTabManagerSpawnTab, SampleModuleTabId2))
        .SetDisplayName(LOCTEXT("SampleModuleTitle2", "SampleModule2"))
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "SessionFrontEnd.Tabs.Tools"))
        .SetGroup(AppMenuGroup);

    TabManager->RegisterTabSpawner(SampleModuleTabId3, FOnSpawnTab::CreateRaw(this, &SP4ToolsWindow::HandleTabManagerSpawnTab, SampleModuleTabId3))
        .SetDisplayName(LOCTEXT("SampleModuleTitle3", "SampleModule3"))
        .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Profiler.Tab"))
        .SetGroup(AppMenuGroup);

    // create tab layout
    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("SampleModuleLayout_v1.2")
                                                        ->AddArea(FTabManager::NewPrimaryArea()
                                                                      ->SetOrientation(Orient_Horizontal)
                                                                      ->Split(
                                                                          // tools
                                                                          FTabManager::NewStack()
                                                                              ->AddTab(SampleModuleTabId0, ETabState::OpenedTab)
                                                                              ->AddTab(SampleModuleTabId1, ETabState::OpenedTab)
                                                                              ->AddTab(SampleModuleTabId2, ETabState::OpenedTab)
                                                                              ->AddTab(SampleModuleTabId3, ETabState::OpenedTab)
                                                                              ->SetForegroundTab(SampleModuleTabId0)));

    // create & initialize main menu
    FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());

    MenuBarBuilder.AddPullDownMenu(LOCTEXT("WindowMenuLabel", "Window"), FText::GetEmpty(), FNewMenuDelegate::CreateStatic(&SP4ToolsWindow::FillWindowMenu, TabManager), "Window");

    ChildSlot[SNew(SVerticalBox)

              + SVerticalBox::Slot().AutoHeight()[MenuBarBuilder.MakeWidget()]

              + SVerticalBox::Slot().FillHeight(1.0f)[TabManager->RestoreFrom(Layout, ConstructUnderWindow).ToSharedRef()]];

    // Tell tab-manager about the multi-box for platforms with a global menu bar
    TabManager->SetMenuMultiBox(MenuBarBuilder.GetMultiBox());
}

/* SP4ToolsWindow implementation
 *****************************************************************************/

void SP4ToolsWindow::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager)
{
    if (!TabManager.IsValid())
    {
        return;
    }

#if !WITH_EDITOR
    FGlobalTabmanager::Get()->PopulateTabSpawnerMenu(MenuBuilder, WorkspaceMenu::GetMenuStructure().GetStructureRoot());
#endif //! WITH_EDITOR

    TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}

/* SP4ToolsWindow callbacks
 *****************************************************************************/

TSharedRef<SDockTab> SP4ToolsWindow::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const
{
    TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;

    TSharedRef<SDockTab> DockTab = SNew(SDockTab).TabRole(ETabRole::PanelTab);

    if (TabIdentifier == SampleModuleTabId0)
    {
        ISampleModule& SampleModule = FModuleManager::LoadModuleChecked<ISampleModule>(TEXT("SampleModule"));
        TabWidget = SampleModule.CreateSampleModuleWindow(DockTab);
    }
    else if (TabIdentifier == SampleModuleTabId1)
    {
        ISampleModule& SampleModule = FModuleManager::LoadModuleChecked<ISampleModule>(TEXT("SampleModule"));
        TabWidget = SampleModule.CreateSampleModuleWindow(DockTab);
    }
    else if (TabIdentifier == SampleModuleTabId2)
    {
        ISampleModule& SampleModule = FModuleManager::LoadModuleChecked<ISampleModule>(TEXT("SampleModule"));
        TabWidget = SampleModule.CreateSampleModuleWindow(DockTab);
    }
    else if (TabIdentifier == SampleModuleTabId3)
    {
        ISampleModule& SampleModule = FModuleManager::LoadModuleChecked<ISampleModule>(TEXT("SampleModule"));
        TabWidget = SampleModule.CreateSampleModuleWindow(DockTab);
    }

    DockTab->SetContent(TabWidget.ToSharedRef());

    return DockTab;
}

#undef LOCTEXT_NAMESPACE
