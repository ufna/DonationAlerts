// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Blueprint/UserWidget.h"
#include "Delegates/DelegateCombinations.h"
#include "Http.h"
#include "IWebSocket.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/SubsystemCollection.h"

#include "DonationAlertsSubsystem.generated.h"

class FJsonObject;
struct FGuid;

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsAuthToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "AuthToken")
	FString access_token;

	UPROPERTY(BlueprintReadWrite, Category = "AuthToken")
	int64 expires_in;

	UPROPERTY(BlueprintReadWrite, Category = "AuthToken")
	FString refresh_token;

public:
	FDonationAlertsAuthToken()
		: expires_in(0){};

	bool IsValid() const { return !access_token.IsEmpty() && !refresh_token.IsEmpty(); }
};

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsUserProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "UserProfile")
	int32 id;

	UPROPERTY(BlueprintReadWrite, Category = "UserProfile")
	FString socket_connection_token;

public:
	FDonationAlertsUserProfile()
		: id(0){};
};

/** Verb used by the request */
UENUM(BlueprintType)
enum class ERequestVerb : uint8
{
	GET,
	POST
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFetchTokenSuccess, const FDonationAlertsAuthToken&, AuthToken);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnRequestError, int32, StatusCode, const FString&, ErrorMessage);

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsEvent
{
	GENERATED_BODY()

	/** Type of the alert. Always donation in this case */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString name;

	/** The name of the user who sent the donation and the alert */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString username;

	/** The message type. The possible values are text for a text messages and audio for an audio messages */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString message_type;

	/** The message sent along with the donation and the alert */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString message;

	/** The donation amount */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	int32 amount;

	/** The currency code (ISO 4217 formatted) */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	FString currency;

	/** A flag indicating whether the alert was shown in the streamer's widget */
	UPROPERTY(BlueprintReadWrite, Category = "Event")
	bool is_shown;

public:
	FDonationAlertsEvent()
		: amount(0)
		, is_shown(false){};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDonationAlertEvent, const FDonationAlertsEvent&, DonationAlertsEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDonationAlertEventStatic, FDonationAlertsEvent);

UCLASS()
class DONATIONALERTS_API UDonationAlertsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDonationAlertsSubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	/** Initialize controller with provided data (used to override project settings) */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts")
	void Initialize(const FString& InAppId);

	/** User will be prompted by the service to authorize or deny the application access to their account */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts")
	void AuthenicateUser(UUserWidget*& BrowserWidget);

	/** Sets AccessToken from OAuth */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts")
	void SetAuthToken(const FDonationAlertsAuthToken& InAuthToken);

	/** Send custom alert to DA server */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts", meta = (AutoCreateRefTerm = "ErrorCallback"))
	void SendCustomAlert(const FOnRequestError& ErrorCallback, const FString& Header = TEXT(""), const FString& Message = TEXT(""), const FString& ImageUrl = TEXT(""), const FString& SoundUrl = TEXT(""));

protected:
	/** Fetch user profile to get keys for socket connection */
	void FetchUserProfile();

	/** Subscribe the channel and obtains connection token */
	void SubscribeCentrifugoChannel(const FString& InChannel);

protected:
	void SendCustomAlert_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);
	void FetchUserProfile_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void SubscribeCentrifugoChannel_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	bool HandleRequestError(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);

protected:
	/** Load save game and extract data */
	void LoadData();

	/** Save cached data or reset one if necessary */
	void SaveData();

	/** Create http request and add API meta */
	TSharedRef<IHttpRequest> CreateHttpRequest(const FString& Url, const FString& BodyContent = TEXT(""), ERequestVerb Verb = ERequestVerb::POST);

	/** Setup auth with bearer token */
	void SetupAuth(TSharedRef<IHttpRequest> HttpRequest);

protected:
	static const FString DonationAlertsEndpoint;
	static const FString DonationAlertsApiEndpoint;
	static const FString DonationAlertsCentrifugoEndpoint;

public:
	/** Get auth url to be opened in browser */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts")
	FString GetAuthUrl() const;

	/** Get cached auth token */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts")
	FDonationAlertsAuthToken GetAuthToken() const;

	/** Event occured when the Donation Alerts event occured (e.g. donation happend on stream) */
	UPROPERTY(BlueprintAssignable, Category = "DonationAlerts")
	FOnDonationAlertEvent DonationAlertsEventHappened;

	/** Callback for login completed event */
	FOnDonationAlertEventStatic DonationAlertsEventHappenedStatic;

protected:
	/** Cached AppId */
	FString AppId;

	/** Cached AuthToken */
	FDonationAlertsAuthToken AuthToken;

	/** Cached user profile */
	FDonationAlertsUserProfile UserProfile;

protected:
	/** Browser widget class to be used when no custom override is used */
	UPROPERTY()
	TSubclassOf<UUserWidget> DefaultBrowserWidgetClass;

protected:
	/** Open connection to Centrifugo with cached keys */
	void OpenCentrifugoSocket();
	void ParseCentrifugoMessage(const FString& InMessage);
	void ConnectDonationChannel(const FString& InChannel, const FString& InToken);

public:
	/** Send user-formed message to socket */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts")
	void SendToSocket(const FString& InMessage);

protected:
	/** Centrifugo socket */
	TSharedPtr<IWebSocket> WebSocket;

	/** Unique message counter per connection */
	int32 MessageId;

	/** Cached centrifugo client id */
	FString ClientId;
};
