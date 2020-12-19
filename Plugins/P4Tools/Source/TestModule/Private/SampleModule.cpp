// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreMinimal.h"

#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWidget.h"

#include "ISampleModule.h"
#include "Widgets/SSampleModuleWindow.h"

/**
 * Implements the FSampleModule module.
 */
class FSampleModule : public ISampleModule
{
public:
    virtual TSharedRef<SWidget> CreateSampleModuleWindow(const TSharedRef<SDockTab>& ConstructUnderMajorTab) override;

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual bool SupportsDynamicReloading() override
    {
        return false;
    }

protected:
};

IMPLEMENT_MODULE(FSampleModule, SampleModule);

/*-----------------------------------------------------------------------------
        FSampleModule
-----------------------------------------------------------------------------*/
void FSampleModule::StartupModule()
{
}

void FSampleModule::ShutdownModule()
{
}

TSharedRef<SWidget> FSampleModule::CreateSampleModuleWindow(const TSharedRef<SDockTab>& ConstructUnderMajorTab)
{

    return SNew(SSampleModuleWindow);
}
