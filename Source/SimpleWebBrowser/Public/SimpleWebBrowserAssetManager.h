// Copyright 2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Materials/Material.h"
#include "UObject/SoftObjectPtr.h"

#include "SimpleWebBrowserAssetManager.generated.h"

class UMaterial;

UCLASS()
class SIMPLEWEBBROWSER_API USimpleWebBrowserAssetManager : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	void LoadDefaultMaterials();

	UMaterial* GetDefaultMaterial() const;
	UMaterial* GetDefaultTranslucentMaterial() const;

protected:
	UPROPERTY()
	TSoftObjectPtr<UMaterial> DefaultMaterial;

	UPROPERTY()
	TSoftObjectPtr<UMaterial> DefaultTranslucentMaterial;
};
