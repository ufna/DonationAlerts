// Copyright 2019 Vladimir Alyamkin. All Rights Reserved.

#include "SimpleWebBrowserAssetManager.h"

#if WITH_EDITOR || PLATFORM_ANDROID || PLATFORM_IOS
#include "WebBrowserTexture.h"
#endif

/**
 * Paths are replaced for particular plugin Content folder
 */
USimpleWebBrowserAssetManager::USimpleWebBrowserAssetManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, DefaultMaterial(FString(TEXT("/DonationAlerts/Browser/M_WebTexture.M_WebTexture")))
	, DefaultTranslucentMaterial(FString(TEXT("/DonationAlerts/Browser/M_WebTexture_Translucent.M_WebTexture_Translucent")))
{
#if WITH_EDITOR || PLATFORM_ANDROID || PLATFORM_IOS
	// Add a hard reference to USimpleWebBrowserTexture, without this the WebBrowserTexture DLL never gets loaded on Windows.
	UWebBrowserTexture::StaticClass();
#endif
};

void USimpleWebBrowserAssetManager::LoadDefaultMaterials()
{
	DefaultMaterial.LoadSynchronous();
	DefaultTranslucentMaterial.LoadSynchronous();
}

UMaterial* USimpleWebBrowserAssetManager::GetDefaultMaterial() const
{
	return DefaultMaterial.Get();
}

UMaterial* USimpleWebBrowserAssetManager::GetDefaultTranslucentMaterial() const
{
	return DefaultTranslucentMaterial.Get();
}
