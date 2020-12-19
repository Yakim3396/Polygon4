// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/SSampleModuleWindow.h"
#include "EditorStyleSet.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SSampleModuleWindow"

SSampleModuleWindow::SSampleModuleWindow()
{
}

SSampleModuleWindow::~SSampleModuleWindow()
{
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSampleModuleWindow::Construct(const FArguments& InArgs)
{
    FText WidgetText = FText::FromString("Sample text");

    ChildSlot[SNew(SBox)
                  .HAlign(HAlign_Center)
                  .VAlign(VAlign_Center)[SNew(STextBlock)
                                             .Text(WidgetText)
                                             .Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 24))
                                             .ColorAndOpacity(FLinearColor::Green)
                                             .ShadowColorAndOpacity(FLinearColor::Black)
                                             .ShadowOffset(FIntPoint(-2, 2))]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
