// Copyright 2019 Vladimir Alyamkin. All Rights Reserved.

#include "SimpleWebBrowserModule.h"

#include "SimpleWebBrowserAssetManager.h"

#include "IWebBrowserSingleton.h"
#include "Materials/Material.h"
#include "Modules/ModuleManager.h"
#include "WebBrowserModule.h"

class FSimpleWebBrowserModule : public ISimpleWebBrowserModule
{
public:
	virtual void StartupModule() override
	{
		if (WebBrowserAssetMgr == nullptr)
		{
			WebBrowserAssetMgr = NewObject<USimpleWebBrowserAssetManager>((UObject*)GetTransientPackage(), NAME_None, RF_Transient | RF_Public);
			WebBrowserAssetMgr->LoadDefaultMaterials();

			IWebBrowserSingleton* WebBrowserSingleton = IWebBrowserModule::Get().GetSingleton();
			if (WebBrowserSingleton)
			{
				WebBrowserSingleton->SetDefaultMaterial(WebBrowserAssetMgr->GetDefaultMaterial());
				WebBrowserSingleton->SetDefaultTranslucentMaterial(WebBrowserAssetMgr->GetDefaultTranslucentMaterial());
			}
		}
	}

	virtual void ShutdownModule() override
	{
	}

private:
	USimpleWebBrowserAssetManager* WebBrowserAssetMgr;
};

IMPLEMENT_MODULE(FSimpleWebBrowserModule, SimpleWebBrowser);
