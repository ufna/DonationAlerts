// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "DonationAlertsController.h"

#include "DonationAlerts.h"
#include "DonationAlertsDefines.h"
#include "DonationAlertsLibrary.h"
#include "DonationAlertsSave.h"
#include "DonationAlertsSettings.h"

#include "Engine/Engine.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Runtime/Launch/Resources/Version.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "FDonationAlertsModule"

const FString UDonationAlertsController::DonationAlertsApiEndpoint(TEXT("https://www.donationalerts.com"));

UDonationAlertsController::UDonationAlertsController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BrowserWidgetFinder(TEXT("/DonationAlerts/Browser/W_AuthBrowser.W_AuthBrowser_C"));
	DefaultBrowserWidgetClass = BrowserWidgetFinder.Class;
}

void UDonationAlertsController::Tick(float DeltaTime)
{
	// Do nothing for now
}

void UDonationAlertsController::Initialize(const FString& InAppId)
{
	// Pre-cache initialization data
	AppId = InAppId;

	// Load cached data
	LoadData();

	UE_LOG(LogDonationAlerts, Log, TEXT("%s: Controller initialized: %s"), *VA_FUNC_LINE, *AppId);
}

void UDonationAlertsController::OpenAuthConsole(UUserWidget*& BrowserWidget)
{
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();

	// Check for user browser widget override
	auto BrowserWidgetClass = (Settings->OverrideBrowserWidgetClass) ? Settings->OverrideBrowserWidgetClass : DefaultBrowserWidgetClass;

	auto MyBrowser = CreateWidget<UUserWidget>(GEngine->GameViewport->GetWorld(), BrowserWidgetClass);
	MyBrowser->AddToViewport(MAX_int32);

	BrowserWidget = MyBrowser;
}

void UDonationAlertsController::SetAuthorizationCode(const FString& InAuthorizationCode)
{
	AuthorizationCode = InAuthorizationCode;
}

void UDonationAlertsController::LoadData()
{
	auto SavedData = UDonationAlertsSave::Load();
	//AccessToken = SavedData.AccessToken;
}

void UDonationAlertsController::SaveData()
{
	//UDonationAlertsSave::Save(FDonationAlertsSaveData(AccessToken));
}

TSharedRef<IHttpRequest> UDonationAlertsController::CreateHttpRequest(const FString& Url, const FString& BodyContent, ERequestVerb Verb)
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();

	HttpRequest->SetURL(Url);

	HttpRequest->SetHeader(TEXT("X-ENGINE"), TEXT("UE4"));
	HttpRequest->SetHeader(TEXT("X-ENGINE-V"), ENGINE_VERSION_STRING);
	HttpRequest->SetHeader(TEXT("X-SDK-V"), DONATIONALERTS_VERSION);

	switch (Verb)
	{
	case ERequestVerb::GET:
		HttpRequest->SetVerb(TEXT("GET"));
		break;

	case ERequestVerb::POST:
		HttpRequest->SetVerb(TEXT("POST"));
		break;

	default:
		unimplemented();
	}

	if (!BodyContent.IsEmpty())
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		HttpRequest->SetContentAsString(BodyContent);
	}

	// @TODO Setup AccessToken

	return HttpRequest;
}

FString UDonationAlertsController::SerializeJson(const TSharedPtr<FJsonObject> DataJson) const
{
	FString JsonContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonContent);
	FJsonSerializer::Serialize(DataJson.ToSharedRef(), Writer);
	return JsonContent;
}

FString UDonationAlertsController::GetAuthUrl() const
{
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();
	return FString::Printf(TEXT("https://www.donationalerts.com/oauth/authorize?client_id=%s&response_type=code&scope=oauth-user-show"), *Settings->AppId);
}

#undef LOCTEXT_NAMESPACE
