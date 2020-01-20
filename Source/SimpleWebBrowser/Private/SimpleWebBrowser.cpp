// Copyright 2019 Vladimir Alyamkin. All Rights Reserved.

#include "SimpleWebBrowser.h"

#include "Async/TaskGraphInterfaces.h"
#include "SWebBrowser.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SimpleWebBrowser"

USimpleWebBrowser::USimpleWebBrowser(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = true;
}

void USimpleWebBrowser::LoadURL(FString NewURL)
{
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->LoadURL(NewURL);
	}
}

FString USimpleWebBrowser::GetUrl() const
{
	if (WebBrowserWidget.IsValid())
	{
		return WebBrowserWidget->GetUrl();
	}

	return FString();
}

void USimpleWebBrowser::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	WebBrowserWidget.Reset();
}

TSharedRef<SWidget> USimpleWebBrowser::RebuildWidget()
{
	// clang-format off
	if (IsDesignTime())
	{
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SimpleBrowser", "Simple Web Browser"))
			];
	}
	else
	{
		WebBrowserWidget = SNew(SWebBrowser)
			.InitialURL(InitialURL)
			.ShowControls(false)
			.SupportsTransparency(bSupportsTransparency)
			.OnUrlChanged(BIND_UOBJECT_DELEGATE(FOnTextChanged, HandleOnUrlChanged))
			.OnBeforePopup(BIND_UOBJECT_DELEGATE(FOnBeforePopupDelegate, HandleOnBeforePopup));

		return WebBrowserWidget.ToSharedRef();
	}
	// clang-format on
}

void USimpleWebBrowser::HandleOnUrlChanged(const FText& InText)
{
	OnUrlChanged.Broadcast(InText);
}

bool USimpleWebBrowser::HandleOnBeforePopup(FString URL, FString Frame)
{
	if (OnBeforePopup.IsBound())
	{
		if (IsInGameThread())
		{
			OnBeforePopup.Broadcast(URL, Frame);
		}
		else
		{
			// clang-format off
			// Retry on the GameThread now
			TWeakObjectPtr<USimpleWebBrowser> WeakThis = this;
			FFunctionGraphTask::CreateAndDispatchWhenReady([WeakThis, URL, Frame]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->HandleOnBeforePopup(URL, Frame);
				}
			}, TStatId(), nullptr, ENamedThreads::GameThread);
			// clang-format on
		}

		return true;
	}

	return false;
}

#if WITH_EDITOR
const FText USimpleWebBrowser::GetPaletteCategory()
{
	return LOCTEXT("SimpleBrowser", "Simple Browser");
}
#endif

#undef LOCTEXT_NAMESPACE
