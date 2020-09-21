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

const FString UDonationAlertsController::DonationAlertsApiEndpoint(TEXT("https://www.donationalerts.com/api/v1"));

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

void UDonationAlertsController::AuthenicateUser(UUserWidget*& BrowserWidget)
{
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();

	// Check for user browser widget override
	auto BrowserWidgetClass = (Settings->OverrideBrowserWidgetClass) ? Settings->OverrideBrowserWidgetClass : DefaultBrowserWidgetClass;

	auto MyBrowser = CreateWidget<UUserWidget>(GEngine->GameViewport->GetWorld(), BrowserWidgetClass);
	MyBrowser->AddToViewport(MAX_int32);

	BrowserWidget = MyBrowser;
}

void UDonationAlertsController::SetAuthToken(const FDonationAlertsAuthToken& InAuthToken)
{
	AuthToken = InAuthToken;
}

void UDonationAlertsController::SendCustomAlert(const FOnRequestError& ErrorCallback, const FString& Header, const FString& Message, const FString& ImageUrl, const FString& SoundUrl)
{
	FString Url = FString::Printf(TEXT("%s/custom_alert?dummy=1"), *DonationAlertsApiEndpoint);

	FString AlertParams;
	if (!Header.IsEmpty())
		AlertParams += FString::Printf(TEXT("&header=%s"), *FGenericPlatformHttp::UrlEncode(Header));
	if (!Message.IsEmpty())
		AlertParams += FString::Printf(TEXT("&message=%s"), *FGenericPlatformHttp::UrlEncode(Message));
	if (!ImageUrl.IsEmpty())
		AlertParams += FString::Printf(TEXT("&image_url=%s"), *FGenericPlatformHttp::UrlEncode(ImageUrl));
	if (!SoundUrl.IsEmpty())
		AlertParams += FString::Printf(TEXT("&sound_url=%s"), *FGenericPlatformHttp::UrlEncode(SoundUrl));

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url + AlertParams);
	SetupAuth(HttpRequest);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDonationAlertsController::SendCustomAlert_HttpRequestComplete, ErrorCallback);
	HttpRequest->ProcessRequest();
}

void UDonationAlertsController::SendCustomAlert_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, ErrorCallback))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogDonationAlerts, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);
}

bool UDonationAlertsController::HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
{
	FString ErrorStr;
	int32 StatusCode = 204;
	FString ResponseStr = TEXT("invalid");

	if (bSucceeded && HttpResponse.IsValid())
	{
		ResponseStr = HttpResponse->GetContentAsString();

		if (!EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
		{
			StatusCode = HttpResponse->GetResponseCode();
			ErrorStr = FString::Printf(TEXT("Invalid response. code=%d error=%s"), HttpResponse->GetResponseCode(), *ResponseStr);

			// Example: {"message" : "Unauthenticated."}
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseStr);
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				static const FString ErrorFieldName = TEXT("message");
				if (JsonObject->HasTypedField<EJson::String>(ErrorFieldName))
				{
					ErrorStr = JsonObject->GetStringField(ErrorFieldName);
				}
				else
				{
					ErrorStr = FString::Printf(TEXT("Can't deserialize error json: no field '%s' found"), *ErrorFieldName);
				}
			}
			else
			{
				ErrorStr = TEXT("Can't deserialize error json");
			}
		}
	}
	else
	{
		ErrorStr = TEXT("No response");
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogDonationAlerts, Warning, TEXT("%s: request failed (%s): %s"), *VA_FUNC_LINE, *ErrorStr, *ResponseStr);
		ErrorCallback.ExecuteIfBound(StatusCode, ErrorStr);
		return true;
	}

	return false;
}

void UDonationAlertsController::LoadData()
{
	auto SavedData = UDonationAlertsSave::Load();
	AuthToken = SavedData.AuthToken;
}

void UDonationAlertsController::SaveData()
{
	UDonationAlertsSave::Save(FDonationAlertsSaveData(AuthToken));
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
	else
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
	}

	return HttpRequest;
}

void UDonationAlertsController::SetupAuth(TSharedRef<IHttpRequest> HttpRequest)
{
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken.access_token));
}

FString UDonationAlertsController::GetAuthUrl() const
{
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();
	return FString::Printf(TEXT("https://www.donationalerts.com/oauth/authorize?client_id=%s&response_type=token&scope=%s&client_secret=%s"),
		*Settings->AppId,
		*FGenericPlatformHttp::UrlEncode(TEXT("oauth-user-show oauth-custom_alert-store")),
		*Settings->AppClientSecret);
}

FDonationAlertsAuthToken UDonationAlertsController::GetAuthToken() const
{
	return AuthToken;
}

#undef LOCTEXT_NAMESPACE
