// Copyright 2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Components/Widget.h"

#include "SimpleWebBrowser.generated.h"

UCLASS()
class SIMPLEWEBBROWSER_API USimpleWebBrowser : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUrlChanged, const FText&, Text);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBeforePopup, FString, URL, FString, Frame);

	/**
	 * Load the specified URL
	 *
	 * @param NewURL New URL to load
	 */
	UFUNCTION(BlueprintCallable, Category = "Simple|Web Browser")
	void LoadURL(FString NewURL);

	/**
	 * Gets the currently loaded URL.
	 *
	 * @return The URL, or empty string if no document is loaded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simple|Web Browser")
	FString GetUrl() const;

	/** Called when the Url changes. */
	UPROPERTY(BlueprintAssignable, Category = "Simple|Web Browser")
	FOnUrlChanged OnUrlChanged;

	/** Called when a popup is about to spawn. */
	UPROPERTY(BlueprintAssignable, Category = "Simple|Web Browser")
	FOnBeforePopup OnBeforePopup;

public:
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	/** URL that the browser will initially navigate to. The URL should include the protocol, eg http:// */
	UPROPERTY(EditAnywhere, Category = Appearance)
	FString InitialURL;

	/** Should the browser window support transparency. */
	UPROPERTY(EditAnywhere, Category = Appearance)
	bool bSupportsTransparency;

protected:
	TSharedPtr<class SWebBrowser> WebBrowserWidget;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	void HandleOnUrlChanged(const FText& Text);
	bool HandleOnBeforePopup(FString URL, FString Frame);
};
