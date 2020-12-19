// Copyright Epic Games, Inc. All Rights Reserved.

#include "P4Tools.h"
#include "P4ToolsStyle.h"
#include "P4ToolsCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

#include "Widgets/SP4ToolsWindow.h"

static const FName P4ToolsTabName("P4Tools");

#define LOCTEXT_NAMESPACE "FP4ToolsModule"

void FP4ToolsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FP4ToolsStyle::Initialize();
	FP4ToolsStyle::ReloadTextures();

	FP4ToolsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FP4ToolsCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FP4ToolsModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FP4ToolsModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(P4ToolsTabName, FOnSpawnTab::CreateRaw(this, &FP4ToolsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FP4ToolsTabTitle", "P4Tools"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FP4ToolsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FP4ToolsStyle::Shutdown();

	FP4ToolsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(P4ToolsTabName);
}

TSharedRef<SDockTab> FP4ToolsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    const TSharedRef<SDockTab> DockTab = SNew(SDockTab).TabRole(ETabRole::MajorTab);
    TSharedRef<SP4ToolsWindow> NewWindow = SNew(SP4ToolsWindow, DockTab, SpawnTabArgs.GetOwnerWindow());
    P4ToolsWindow = NewWindow;
    DockTab->SetContent(NewWindow);
    return DockTab;
}

void FP4ToolsModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(P4ToolsTabName);
}

void FP4ToolsModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FP4ToolsCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FP4ToolsCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FP4ToolsModule, P4Tools)
