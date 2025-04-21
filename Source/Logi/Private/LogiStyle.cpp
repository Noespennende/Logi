// Copyright Epic Games, Inc. All Rights Reserved.

#include "LogiStyle.h"
#include "Logi.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FLogiStyle::StyleInstance = nullptr;

void FLogiStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		
		UE_LOG(LogTemp, Error, TEXT("FLogiStyle::Initialize called"));
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);

		FSlateIcon(FLogiStyle::GetStyleSetName(), "Logi.PluginAction");
		FLogiStyle::ReloadTextures();
	}
}

void FLogiStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLogiStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LogiStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

TSharedRef< FSlateStyleSet > FLogiStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LogiStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("Logi")->GetBaseDir() / TEXT("Resources"));

	Style->Set("Logi.PluginAction", new IMAGE_BRUSH("Icon64", Icon64x64));

	return Style;
}

void FLogiStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLogiStyle::Get()
{
	return *StyleInstance;
}
