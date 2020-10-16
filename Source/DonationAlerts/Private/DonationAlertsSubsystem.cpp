// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "DonationAlertsSubsystem.h"

#include "DonationAlerts.h"
#include "DonationAlertsDefines.h"
#include "DonationAlertsLibrary.h"
#include "DonationAlertsSave.h"
#include "DonationAlertsSettings.h"

#include "Engine/Engine.h"
#include "IWebSocket.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Runtime/Launch/Resources/Version.h"
#include "UObject/ConstructorHelpers.h"
#include "WebSocketsModule.h"

#define LOCTEXT_NAMESPACE "FDonationAlertsModule"

const FString UDonationAlertsSubsystem::DonationAlertsEndpoint(TEXT("https://www.donationalerts.com"));
const FString UDonationAlertsSubsystem::DonationAlertsApiEndpoint(TEXT("https://www.donationalerts.com/api/v1"));
const FString UDonationAlertsSubsystem::DonationAlertsCentrifugoEndpoint(TEXT("wss://centrifugo.donationalerts.com/connection/websocket"));

UDonationAlertsSubsystem::UDonationAlertsSubsystem()
	: UGameInstanceSubsystem()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> BrowserWidgetFinder(TEXT("/DonationAlerts/Browser/W_AuthBrowser.W_AuthBrowser_C"));
	DefaultBrowserWidgetClass = BrowserWidgetFinder.Class;

	MessageId = 1;
}

void UDonationAlertsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize subsystem with project identifier provided by user
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();
	Initialize(Settings->AppId);

	UE_LOG(LogDonationAlerts, Log, TEXT("%s: LogDonationAlerts subsystem initialized"), *VA_FUNC_LINE);
}

void UDonationAlertsSubsystem::Deinitialize()
{
	if (WebSocket.IsValid())
	{
		WebSocket->Close();
	}

	Super::Deinitialize();
}

void UDonationAlertsSubsystem::Initialize(const FString& InAppId)
{
	// Pre-cache initialization data
	AppId = InAppId;

	// Load cached data
	LoadData();

	UE_LOG(LogDonationAlerts, Log, TEXT("%s: Controller initialized: %s"), *VA_FUNC_LINE, *AppId);
}

void UDonationAlertsSubsystem::AuthenicateUser(UUserWidget*& BrowserWidget)
{
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();

	// Check for user browser widget override
	auto BrowserWidgetClass = (Settings->OverrideBrowserWidgetClass) ? Settings->OverrideBrowserWidgetClass : DefaultBrowserWidgetClass;

	auto MyBrowser = CreateWidget<UUserWidget>(GEngine->GameViewport->GetWorld(), BrowserWidgetClass);
	MyBrowser->AddToViewport(MAX_int32);

	BrowserWidget = MyBrowser;
}

void UDonationAlertsSubsystem::SetAuthToken(const FDonationAlertsAuthToken& InAuthToken)
{
	AuthToken = InAuthToken;

	// Automatically fetch user profile each time we've got the auth token
	FetchUserProfile();
}

void UDonationAlertsSubsystem::SendCustomAlert(const FOnRequestError& ErrorCallback, const FString& Header, const FString& Message, const FString& ImageUrl, const FString& SoundUrl)
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
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDonationAlertsSubsystem::SendCustomAlert_HttpRequestComplete, ErrorCallback);
	HttpRequest->ProcessRequest();
}

void UDonationAlertsSubsystem::FetchUserProfile()
{
	FString Url = FString::Printf(TEXT("%s/user/oauth"), *DonationAlertsApiEndpoint);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, FString(), ERequestVerb::GET);
	SetupAuth(HttpRequest);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDonationAlertsSubsystem::FetchUserProfile_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UDonationAlertsSubsystem::SubscribeCentrifugoChannel(const TArray<FString>& InChannels)
{
	FString ChannelsStr;
	for (auto& Channel : InChannels)
	{
		ChannelsStr += TEXT("\"") + Channel + TEXT("\",");
	}

	// Remove last quote
	if (!ChannelsStr.IsEmpty())
	{
		ChannelsStr.LeftChopInline(1);
	}

	FString Url = FString::Printf(TEXT("%s/centrifuge/subscribe"), *DonationAlertsApiEndpoint);
	FString PostContent = FString::Printf(TEXT("{\"channels\":[%s], \"client\":\"%s\"}"), *ChannelsStr, *ClientId);

	TSharedRef<IHttpRequest> HttpRequest = CreateHttpRequest(Url, PostContent);
	SetupAuth(HttpRequest);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UDonationAlertsSubsystem::SubscribeCentrifugoChannel_HttpRequestComplete);
	HttpRequest->ProcessRequest();
}

void UDonationAlertsSubsystem::SendCustomAlert_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, ErrorCallback))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogDonationAlerts, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);
}

void UDonationAlertsSubsystem::FetchUserProfile_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, FOnRequestError()))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogDonationAlerts, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);

	FString ErrorStr;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseStr);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		static const FString DataFieldName = TEXT("data");
		if (JsonObject->HasTypedField<EJson::Object>(DataFieldName))
		{
			UserProfile.id = JsonObject->GetObjectField(DataFieldName)->GetNumberField(TEXT("id"));
			UserProfile.socket_connection_token = JsonObject->GetObjectField(DataFieldName)->GetStringField(TEXT("socket_connection_token"));

			OpenCentrifugoSocket();

			if (UserProfile.socket_connection_token.IsEmpty())
			{
				ErrorStr = TEXT("Invalid user profile data");
			}
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Can't deserialize error json: no field '%s' found"), *DataFieldName);
		}
	}
	else
	{
		ErrorStr = TEXT("Can't deserialize json");
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogDonationAlerts, Error, TEXT("%s: %s"), *VA_FUNC_LINE, *ErrorStr);
	}
}

void UDonationAlertsSubsystem::SubscribeCentrifugoChannel_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (HandleRequestError(HttpRequest, HttpResponse, bSucceeded, FOnRequestError()))
	{
		return;
	}

	FString ResponseStr = HttpResponse->GetContentAsString();
	UE_LOG(LogDonationAlerts, Verbose, TEXT("%s: Response: %s"), *VA_FUNC_LINE, *ResponseStr);

	FString ErrorStr;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*ResponseStr);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		static const FString DataFieldName = TEXT("channels");
		if (JsonObject->HasTypedField<EJson::Array>(DataFieldName))
		{
			auto ChannelsArray = JsonObject->GetArrayField(DataFieldName);

			bool bGotToken = false;
			for (auto& Channel : ChannelsArray)
			{
				auto ChannelObject = Channel->AsObject();
				if (ChannelObject.IsValid())
				{
					ConnectDonationChannel(ChannelObject->GetStringField("channel"), ChannelObject->GetStringField("token"));
					bGotToken = true;
				}
			}

			if (!bGotToken)
			{
				ErrorStr = TEXT("Invalid channel data");
			}
		}
		else
		{
			ErrorStr = FString::Printf(TEXT("Can't deserialize error json: no field '%s' found"), *DataFieldName);
		}
	}
	else
	{
		ErrorStr = TEXT("Can't deserialize json");
	}

	if (!ErrorStr.IsEmpty())
	{
		UE_LOG(LogDonationAlerts, Error, TEXT("%s: %s"), *VA_FUNC_LINE, *ErrorStr);
	}
}

bool UDonationAlertsSubsystem::HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback)
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

void UDonationAlertsSubsystem::LoadData()
{
	auto SavedData = UDonationAlertsSave::Load();
	AuthToken = SavedData.AuthToken;
}

void UDonationAlertsSubsystem::SaveData()
{
	UDonationAlertsSave::Save(FDonationAlertsSaveData(AuthToken));
}

TSharedRef<IHttpRequest> UDonationAlertsSubsystem::CreateHttpRequest(const FString& Url, const FString& BodyContent, ERequestVerb Verb)
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

void UDonationAlertsSubsystem::SetupAuth(TSharedRef<IHttpRequest> HttpRequest)
{
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken.access_token));
}

FString UDonationAlertsSubsystem::GetAuthUrl() const
{
	const UDonationAlertsSettings* Settings = FDonationAlertsModule::Get().GetSettings();
	return FString::Printf(TEXT("%s/oauth/authorize?client_id=%s&response_type=token&scope=%s&client_secret=%s"),
		*DonationAlertsEndpoint,
		*Settings->AppId,
		*FGenericPlatformHttp::UrlEncode(TEXT("oauth-user-show oauth-donation-subscribe oauth-donation-index oauth-custom_alert-store")),
		*Settings->AppClientSecret);
}

FDonationAlertsAuthToken UDonationAlertsSubsystem::GetAuthToken() const
{
	return AuthToken;
}

void UDonationAlertsSubsystem::OpenCentrifugoSocket()
{
	const FString ServerProtocol = TEXT("wss");
	TMap<FString, FString> UpgradeHeaders;

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(DonationAlertsCentrifugoEndpoint, ServerProtocol, UpgradeHeaders);

	WebSocket->OnConnected().AddWeakLambda(this, [this]() -> void {
		UE_LOG(LogDonationAlerts, Warning, TEXT("%s: Socket connected"), *VA_FUNC_LINE);

		SendToSocket(FString::Printf(TEXT("{\"params\":{\"token\":\"%s\"},\"id\":%d}"), *UserProfile.socket_connection_token, MessageId));
	});

	WebSocket->OnConnectionError().AddLambda([](const FString& Error) -> void {
		UE_LOG(LogDonationAlerts, Error, TEXT("%s: Socket error: %s"), *VA_FUNC_LINE, *Error);
	});

	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean) -> void {
		UE_LOG(LogDonationAlerts, Log, TEXT("%s: Socket closed (%d): %s"), *VA_FUNC_LINE, StatusCode, *Reason);
	});

	WebSocket->OnMessage().AddWeakLambda(this, [this](const FString& Message) -> void {
		UE_LOG(LogDonationAlerts, Verbose, TEXT("%s: Socket received: %s"), *VA_FUNC_LINE, *Message);

		ParseCentrifugoMessage(Message);
	});

	WebSocket->OnMessageSent().AddLambda([](const FString& Message) -> void {
		UE_LOG(LogDonationAlerts, Verbose, TEXT("%s: Socket sent: %s"), *VA_FUNC_LINE, *Message);
	});

	WebSocket->Connect();
}

void UDonationAlertsSubsystem::ParseCentrifugoMessage(const FString& InMessage)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*InMessage);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		static const FString ResultFieldName = TEXT("result");
		if (JsonObject->HasTypedField<EJson::Object>(ResultFieldName))
		{
			TSharedPtr<FJsonObject> ResultJson = JsonObject->GetObjectField(ResultFieldName);

			static const FString ClientFieldName = TEXT("client");
			static const FString ChannelFieldName = TEXT("channel");
			static const FString DataFieldName = TEXT("data");

			// Check client auth
			if (ResultJson->HasTypedField<EJson::String>(ClientFieldName))
			{
				ClientId = ResultJson->GetStringField(ClientFieldName);

				// We've connected to server, so now we should join a channel
				FString AlertsChannel = FString::Printf(TEXT("$alerts:donation_%d"), UserProfile.id);
				FString PollsChannel = FString::Printf(TEXT("$polls:poll_%d"), UserProfile.id);
				FString GoalsChannel = FString::Printf(TEXT("$goals:goal_%d"), UserProfile.id);

				SubscribeCentrifugoChannel({AlertsChannel, PollsChannel, GoalsChannel});
			}
			else if (ResultJson->HasTypedField<EJson::String>(ChannelFieldName))
			{
				// That's the matryoshka data structure
				if (auto DataJson = ResultJson->GetObjectField(DataFieldName))
				{
					// Now get result.data.data
					if (DataJson->HasTypedField<EJson::Object>(DataFieldName))
					{
						auto SecondLayerDataJson = DataJson->GetObjectField(DataFieldName);

						// Select channel now
						FString ChannelName = ResultJson->GetStringField(ChannelFieldName);
						if (ChannelName.StartsWith(TEXT("$alerts")))
						{
							FDonationAlertsDonationEvent DAEvent;
							if (!FJsonObjectConverter::JsonObjectToUStruct(SecondLayerDataJson.ToSharedRef(), FDonationAlertsDonationEvent::StaticStruct(), &DAEvent))
							{
								UE_LOG(LogDonationAlerts, Error, TEXT("%s: Can't convert data to struct"), *VA_FUNC_LINE);
								return;
							}

							OnDonationEvent.Broadcast(DAEvent);
							OnDonationEventStatic.Broadcast(DAEvent);
						}
						else if (ChannelName.StartsWith(TEXT("$polls")))
						{
							FDonationAlertsPollEvent DAEvent;
							if (!FJsonObjectConverter::JsonObjectToUStruct(SecondLayerDataJson.ToSharedRef(), FDonationAlertsPollEvent::StaticStruct(), &DAEvent))
							{
								UE_LOG(LogDonationAlerts, Error, TEXT("%s: Can't convert data to struct"), *VA_FUNC_LINE);
								return;
							}

							OnPollEvent.Broadcast(DAEvent);
							OnPollEventStatic.Broadcast(DAEvent);
						}
						else if (ChannelName.StartsWith(TEXT("$goals")))
						{
							FDonationAlertsGoalEvent DAEvent;
							if (!FJsonObjectConverter::JsonObjectToUStruct(SecondLayerDataJson.ToSharedRef(), FDonationAlertsGoalEvent::StaticStruct(), &DAEvent))
							{
								UE_LOG(LogDonationAlerts, Error, TEXT("%s: Can't convert data to struct"), *VA_FUNC_LINE);
								return;
							}

							OnGoalEvent.Broadcast(DAEvent);
							OnGoalEventStatic.Broadcast(DAEvent);
						}
						else
						{
							UE_LOG(LogDonationAlerts, Warning, TEXT("%s: Unknown channel: %s"), *VA_FUNC_LINE, *ChannelName);
						}
					}
				}
			}
		}
	}
}

void UDonationAlertsSubsystem::ConnectDonationChannel(const FString& InChannel, const FString& InToken)
{
	TSharedPtr<FJsonObject> RequestParamsJson = MakeShareable(new FJsonObject());
	RequestParamsJson->SetStringField(TEXT("channel"), InChannel);
	RequestParamsJson->SetStringField(TEXT("token"), InToken);

	FString ParamsContent;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ParamsContent);
	FJsonSerializer::Serialize(RequestParamsJson.ToSharedRef(), Writer);

	SendToSocket(FString::Printf(TEXT("{\"params\":%s,\"id\":%d,\"method\":1}"), *ParamsContent, MessageId));
}

void UDonationAlertsSubsystem::SendToSocket(const FString& InMessage)
{
	if (!WebSocket.IsValid() || !WebSocket->IsConnected())
	{
		UE_LOG(LogDonationAlerts, Error, TEXT("%s: Socket is not connected. Trying to send: %s"), *VA_FUNC_LINE, *InMessage);
		return;
	}

	WebSocket->Send(InMessage);

	// Each send increments message count
	MessageId++;
}

#undef LOCTEXT_NAMESPACE
