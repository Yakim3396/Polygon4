#include "DBTool.h"

#include "Landscape.h"
#include "LandscapeEdit.h"
#include "LandscapeInfo.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "DBToolStyle.h"
#include "DBToolCommands.h"

#include "IPluginManager.h"
#include "LevelEditor.h"
#include "EngineUtils.h"

#include "Widgets/SDBToolTreeView.h"
#include "Widgets/SDBToolTableView.h"

#include <Polygon4/Memory.h>
//POLYGON4_UNREAL_MEMORY_STUB

#include <Polygon4/DataManager/Database.h>
#include <Polygon4/DataManager/Schema.h>
#include <Polygon4/DataManager/Storage.h>
#include <Polygon4/DataManager/StorageImpl.h>
#include <Polygon4/DataManager/Types.h>

#include <Polygon4/Game/P4Glider.h>
#include <Polygon4/Game/P4Mechanoid.h>
#include <Polygon4/Game/P4MapBuilding.h>
#include <Polygon4/Game/P4MapObject.h>

#include <Polygon4/Mechanoid.h>

#include <memory>
//#include <optional>!!!
#include <tuple>

static const FName DBToolTabName("DBTool");
FDBToolModule *GDBToolModule;

#define LOCTEXT_NAMESPACE "FDBToolModule"

void FDBToolModule::StartupModule()
{
    GDBToolModule = this;

    FDBToolStyle::Initialize();
    FDBToolStyle::ReloadTextures();

    FDBToolCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        FDBToolCommands::Get().OpenPluginWindow,
        FExecuteAction::CreateRaw(this, &FDBToolModule::PluginButtonClicked),
        FCanExecuteAction());

    PluginCommands->MapAction(
        FDBToolCommands::Get().SaveDB,
        FExecuteAction::CreateLambda([this]() { SaveDB(); }),
        FCanExecuteAction());
    PluginCommands->MapAction(
        FDBToolCommands::Get().ReloadDB,
        FExecuteAction::CreateRaw(this, &FDBToolModule::ReloadDB),
        FCanExecuteAction());

    //
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    {
        TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
        MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FDBToolModule::AddMenuExtension));

        LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
    }
    {
        TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
        ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FDBToolModule::AddToolbarExtension));

        LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
    }
    auto &TabSpawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DBToolTabName, FOnSpawnTab::CreateRaw(this, &FDBToolModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("FDBToolTabTitle", "DB Tool"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
    TabSpawnerEntry = TabSpawner.AsSpawnerEntry();

    // toolbar
    ToolBarBuilder = MakeShareable(new FToolBarBuilder(PluginCommands, FMultiBoxCustomization::None));
    {
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().SaveDB);
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().ReloadDB);
        ToolBarBuilder->AddSeparator();
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().AddRecord);
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().DeleteRecord);
        ToolBarBuilder->AddSeparator();
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().LoadMapMechanoidsFromDB);
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().SaveMapMechanoidsToDB);
        ToolBarBuilder->AddSeparator();
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().LoadMapObjectsFromDB);
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().SaveMapObjectsToDB);
        ToolBarBuilder->AddSeparator();
        ToolBarBuilder->AddToolBarButton(FDBToolCommands::Get().LoadMapHeightmap);
        ToolBarBuilder->AddSeparator();
    }

    // db load
    FString DBFile = IPluginManager::Get().FindPlugin("DBTool")->GetBaseDir() / TEXT("../../Mods/db.sqlite");
    try
    {
        database = std::make_shared<polygon4::Database>(TCHAR_TO_ANSI(DBFile.GetCharArray().GetData()));
    }
    catch (std::exception &e)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot load database: %s"), e.what());
        return;
    }
    if (!LoadDB())
        return;
}

void FDBToolModule::ShutdownModule()
{
    FDBToolStyle::Shutdown();
    FDBToolCommands::Unregister();
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DBToolTabName);
}

TSharedRef<SDockTab> FDBToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
    std::shared_ptr<polygon4::TreeItem> RootItem;
    if (storage)
        RootItem = storage->printTree();

    SAssignNew(TableView, SDBToolTableView).Storage(storage);
    SAssignNew(Tab, SDockTab)
        .TabRole(ETabRole::NomadTab)
        .OnCanCloseTab_Lambda([this]()
        {
            if (dataChanged)
            {
                TSharedPtr<FText> title = MakeShareable(new FText(LOCTEXT("ExitMessageTitle", "Confirm exit")));
                auto r = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("ExitMessage", "You have unsaved changes.\n Do you really want to exit?"), title.Get());
                if (r == EAppReturnType::No)
                    return false;
            }
            SetDataCommitted();
            Tab.Reset();
            TableView.Reset();
            return true;
        })
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Top)
            .AutoHeight()
            [
                ToolBarBuilder->MakeWidget()
            ]
            + SVerticalBox::Slot()
            .Padding(0, 5, 0, 0)
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SHorizontalBox)
                // Tree view
                + SHorizontalBox::Slot()
                .FillWidth(0.4)
                .VAlign(VAlign_Fill)
                [
                    SNew(SBorder)
                    .Padding(10)
                    [
                        SAssignNew(TreeView, SDBToolTreeView)
                        .RootItem(RootItem)
                        .TableView(TableView)
                        .Storage(storage)
                    ]
                ]
                // Table
                + SHorizontalBox::Slot()
                //.FillWidth(0.75)
                .Padding(5, 0, 0, 0)
                .VAlign(VAlign_Fill)
                [
                    SNew(SBorder)
                    .Padding(10)
                    [
                        TableView.ToSharedRef()
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .Padding(0, 5, 0, 0)
            .HAlign(HAlign_Fill)
            .AutoHeight()
            [
                SNew(SBorder)
                [
                    SAssignNew(StatusBar, STextBlock)
                    .Text(LOCTEXT("DBDataCommitted", "There is no any changes"))
                ]
            ]
        ];
    SetDataCommitted();

    PluginCommands->MapAction(
        FDBToolCommands::Get().AddRecord,
        FExecuteAction::CreateRaw(TreeView.Get(), &SDBToolTreeView::AddRecord),
        FCanExecuteAction());
    PluginCommands->MapAction(
        FDBToolCommands::Get().DeleteRecord,
        FExecuteAction::CreateRaw(TreeView.Get(), &SDBToolTreeView::DeleteRecord),
        FCanExecuteAction());

    PluginCommands->MapAction(
        FDBToolCommands::Get().LoadMapMechanoidsFromDB,
        FExecuteAction::CreateRaw(this, &FDBToolModule::LoadMapMechanoidsFromDB),
        FCanExecuteAction());
    PluginCommands->MapAction(
        FDBToolCommands::Get().SaveMapMechanoidsToDB,
        FExecuteAction::CreateRaw(this, &FDBToolModule::SaveMapMechanoidsToDB),
        FCanExecuteAction());

    PluginCommands->MapAction(
        FDBToolCommands::Get().LoadMapObjectsFromDB,
        FExecuteAction::CreateRaw(this, &FDBToolModule::LoadMapObjectsFromDB),
        FCanExecuteAction());
    PluginCommands->MapAction(
        FDBToolCommands::Get().SaveMapObjectsToDB,
        FExecuteAction::CreateRaw(this, &FDBToolModule::SaveMapObjectsToDB),
        FCanExecuteAction());

    PluginCommands->MapAction(
        FDBToolCommands::Get().LoadMapHeightmap,
        FExecuteAction::CreateRaw(this, &FDBToolModule::LoadMapHeightmap),
        FCanExecuteAction());

    return Tab.ToSharedRef();
}

void FDBToolModule::PluginButtonClicked()
{
    FGlobalTabmanager::Get()->InvokeTab(DBToolTabName);
}

void FDBToolModule::AddMenuExtension(FMenuBuilder& Builder)
{
    Builder.AddMenuEntry(FDBToolCommands::Get().OpenPluginWindow);
}

void FDBToolModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
    Builder.AddToolBarButton(FDBToolCommands::Get().OpenPluginWindow);
}

bool FDBToolModule::LoadDB()
{
    if (dataChanged)
    {
        TSharedPtr<FText> title = MakeShareable(new FText(LOCTEXT("LoadMessageTitle", "Confirm load")));
        auto r = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("LoadMessage", "You have unsaved changes.\n Do you really want to load data from the database?"), title.Get());
        if (r == EAppReturnType::No)
            return false;
    }

    try
    {
        storage = polygon4::initStorage(database);
        storage->load();

        storage->getSettings().flags.set(polygon4::gfDbTool);

#define REPLACE(a, t) \
    for (auto &v : storage->a) \
        v.second->replace<t>(v.second)

        REPLACE(mechanoids, polygon4::Mechanoid);

        SetDataCommitted();
    }
    catch (std::exception &e)
    {
        storage.reset();
        UE_LOG(LogTemp, Error, TEXT("Cannot load storage: %s"), e.what());
        return false;
    }
    if (TreeView.IsValid())
        TreeView->Reset(storage->printTree());
    if (TableView.IsValid())
        TableView->ResetTable();
    return true;
}

bool FDBToolModule::SaveDB()
{
    if (!storage)
        return false;
    try
    {
        storage->save();
        SetDataCommitted();
    }
    catch (std::exception &e)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot save database: %s"), e.what());
        return false;
    }
    return true;
}

void FDBToolModule::ReloadDB()
{
    LoadDB();
}

void error_text(const FText &Title, const FText &Text)
{
    FMessageDialog::Open(EAppMsgType::Ok, Text, &Title);
}

// make optional<>
std::unique_ptr<std::tuple<polygon4::detail::Modification*, polygon4::IdPtr<polygon4::detail::Map>>>
GetCurrentMap(SDBToolTreeView *TreeView)
{
    using namespace polygon4::detail;

    auto error = []()
    {
        TSharedPtr<FText> title = MakeShareable(new FText(LOCTEXT("SaveLoadMapObjectsError", "Select a valid modification")));
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveLoadMapObjectsErrorMessage", "Please, select a valid modification or its child in the object tree"), title.Get());
    };

    auto Items = TreeView->GetSelectedItems();
    if (Items.Num() == 0)
    {
        error();
        return nullptr;
    }
    auto Item0 = Items[0];
    if (!Item0.IsValid())
    {
        error();
        return nullptr;
    }
    auto Item = Item0.Get();
    if (!Item->P4Item || !Item->P4Item->object)
    {
        error();
        return nullptr;
    }
    while (Item->Parent)
    {
        if (Item->P4Item->object && Item->P4Item->object->getType() == EObjectType::Modification)
            break;
        Item = Item->Parent;
    }
    if (!Item->Parent)
    {
        error();
        return nullptr;
    }
    if (!Item->P4Item->object || Item->P4Item->object->getType() != EObjectType::Modification)
    {
        error();
        return nullptr;
    }

    auto MapName = GWorld->GetOuter()->GetName();
    auto modification = (Modification *)Item->P4Item->object;
    IdPtr<Map> map;
    polygon4::String name = MapName.GetCharArray().GetData();
    for (auto &m : modification->maps)
    {
        if (m->map->resource == name)
        {
            map = m->map;
            break;
        }
    }
    if (!map)
    {
        error_text(
            LOCTEXT("SaveLoadMapObjectsMapNotFound", "Map is not found"),
            FText::Format(LOCTEXT("SaveLoadMapObjectsMapNotFoundText", "This map '{0}' is not included in selected modification"), FText::FromString(MapName)));
        return nullptr;
    }

    return std::make_unique<std::tuple<polygon4::detail::Modification*, polygon4::IdPtr<polygon4::detail::Map>>>(modification, map);
}

void FDBToolModule::SaveMapMechanoidsToDB()
{
    using namespace polygon4::detail;

    auto p = GetCurrentMap(TreeView.Get());
    if (!p)
        return;
    auto map = std::get<IdPtr<Map>>(*p);
    auto modification = std::get<Modification*>(*p);

    TMap<FString, Mechanoid*> mechanoids;
    for (auto &m : storage->mechanoids)
    {
        FString s = m.second->text_id;
        if (mechanoids.Contains(s))
        {
            error_text(
                LOCTEXT("SaveMapMechanoidsDuplicateErrorText", "Select valid modification"),
                FText::Format(LOCTEXT("SaveMapMechanoidsDuplicateError", "There's duplicate text_id key '{0}'"), FText::FromString(s)));
            return;
        }
        mechanoids.Add(s, m.second);
    }

    bool updated = false;
    for (TActorIterator<AP4Glider> Itr(GWorld); Itr; ++Itr)
    {
        if (mechanoids.Contains(Itr->Data.TextID))
        {
            auto m = mechanoids[Itr->Data.TextID];
            m->modification = modification;
            m->map = map;
            auto Loc = Itr->GetActorLocation();
            auto Rot = Itr->GetActorRotation();
            m->x = Loc.X;
            m->y = Loc.Y;
            m->z = Loc.Z;
            m->pitch = Rot.Pitch;
            m->yaw = Rot.Yaw;
            m->roll = Rot.Roll;
            m->map = map;

            auto full_name = Itr->GetClass()->GetFullName();

            Itr->Destroy();
            updated = true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Mechanoid: TextID: '%s', Name: '%s' is not found in the database!"),
                Itr->Data.TextID.GetCharArray().GetData(), Itr->GetName().GetCharArray().GetData());
        }
    }
    if (updated)
        TableView->ResetTable();
}

void FDBToolModule::LoadMapMechanoidsFromDB()
{
    using namespace polygon4::detail;

    auto p = GetCurrentMap(TreeView.Get());
    if (!p)
        return;
    auto map = std::get<IdPtr<Map>>(*p);
    auto modification = std::get<Modification*>(*p);

    auto &mechanoids = modification->mechanoids;
    std::unordered_set<Mechanoid*> mapMechanoids;
    for (auto &m : mechanoids)
    {
        if (m->map == map)
            mapMechanoids.insert(m);
    }

    for (auto &m : mapMechanoids)
    {
        spawn(m, GWorld);
    }
}

void FDBToolModule::SaveMapObjectsToDB()
{
    using namespace polygon4::detail;

    auto p = GetCurrentMap(TreeView.Get());
    if (!p)
        return;
    auto map = std::get<IdPtr<Map>>(*p);
    auto modification = std::get<Modification*>(*p);

    /*TMap<FString, MapBuilding*> objects;
    for (auto &m : storage->mapBuildings)
    {
        FString s = m.second->text_id;
        if (objects.Contains(s))
        {
            error_text(
                LOCTEXT("SaveMapMechanoidsDuplicateErrorText", "Select valid modification"),
                FText::Format(LOCTEXT("SaveMapMechanoidsDuplicateError", "There's duplicate text_id key '{0}'"), FText::FromString(s)));
            return;
        }
        objects.Add(s, m.second);
    }

    bool updated = false;
    for (TActorIterator<AP4Building> Itr(GWorld); Itr; ++Itr)
    {
        if (objects.Contains(Itr->Data.TextID))
        {
            auto m = objects[Itr->Data.TextID];
            m->modification = modification;
            m->map = map;
            auto Loc = Itr->GetActorLocation();
            auto Rot = Itr->GetActorRotation();
            m->x = Loc.X;
            m->y = Loc.Y;
            m->z = Loc.Z;
            m->pitch = Rot.Pitch;
            m->yaw = Rot.Yaw;
            m->roll = Rot.Roll;
            m->map = map;

            auto full_name = Itr->GetClass()->GetFullName();

            Itr->Destroy();
            updated = true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Mechanoid: TextID: '%s', Name: '%s' is not found in the database!"),
                Itr->Data.TextID.GetCharArray().GetData(), Itr->GetName().GetCharArray().GetData());
        }
    }
    if (updated)
        TableView->ResetTable();*/
}

void FDBToolModule::LoadMapObjectsFromDB()
{
    using namespace polygon4::detail;

    auto p = GetCurrentMap(TreeView.Get());
    if (!p)
        return;
    auto map = std::get<IdPtr<Map>>(*p);
    auto modification = std::get<Modification*>(*p);

    auto spawn = [](const auto &a)
    {
        for (auto &o : a)
            ::spawn(o, GWorld);
    };

    spawn(map->buildings);
    spawn(map->objects);
}

void FDBToolModule::LoadMapHeightmap()
{
    TActorIterator<ALandscape> landscapeIterator(GWorld);
    if (!landscapeIterator)
        return;
    ALandscape* Landscape = *landscapeIterator;
    auto LandscapeInfo = Landscape->GetLandscapeInfo();

    int32 MinX = MAX_int32;
    int32 MinY = MAX_int32;
    int32 MaxX = -MAX_int32;
    int32 MaxY = -MAX_int32;

    if (!LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY))
        return;

    FLandscapeEditDataInterface LandscapeEdit(LandscapeInfo);

    TArray<uint8> HeightData;
    HeightData.AddZeroed((MaxX - MinX + 1) * (MaxY - MinY + 1) * sizeof(uint16));
    LandscapeEdit.GetHeightDataFast(MinX, MinY, MaxX, MaxY, (uint16*)HeightData.GetData(), 0);
}

void FDBToolModule::SetDataChanged()
{
    dataChanged = true;
    if (StatusBar.IsValid())
        StatusBar->SetText(LOCTEXT("DBDataChanged", "Data is changed"));
}

void FDBToolModule::SetDataCommitted()
{
    dataChanged = false;
    if (StatusBar.IsValid())
        StatusBar->SetText(LOCTEXT("DBDataCommitted", "There is no any changes"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDBToolModule, DBTool)
