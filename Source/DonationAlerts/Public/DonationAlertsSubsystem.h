// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "DonationAlertsTypes.h"

#include "Blueprint/UserWidget.h"
#include "Delegates/DelegateCombinations.h"
#include "Http.h"
#include "IWebSocket.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/SubsystemCollection.h"

#include "DonationAlertsSubsystem.generated.h"

class FJsonObject;
struct FGuid;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFetchTokenSuccess, const FDonationAlertsAuthToken&, AuthToken);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnRequestError, int32, StatusCode, const FString&, ErrorMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDADonationEvent, const FDonationAlertsDonationEvent&, DonationEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDADonationEventStatic, FDonationAlertsDonationEvent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDAGoalEvent, const FDonationAlertsGoalEvent&, GoalEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDAGoalEventStatic, FDonationAlertsGoalEvent);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDAPollEvent, const FDonationAlertsPollEvent&, PollEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDAPollEventStatic, FDonationAlertsPollEvent);

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
	void SubscribeCentrifugoChannel(const TArray<FString>& InChannels);

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
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateHttpRequest(const FString& Url, const FString& BodyContent = TEXT(""), ERequestVerb Verb = ERequestVerb::POST);

	/** Setup auth with bearer token */
	void SetupAuth(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest);

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
	FOnDADonationEvent OnDonationEvent;
	FOnDADonationEventStatic OnDonationEventStatic;

	/** Event occured when the goal on stream achieved */
	UPROPERTY(BlueprintAssignable, Category = "DonationAlerts")
	FOnDAGoalEvent OnGoalEvent;
	FOnDAGoalEventStatic OnGoalEventStatic;

	/** Event occured when the poll on stream finished */
	UPROPERTY(BlueprintAssignable, Category = "DonationAlerts")
	FOnDAPollEvent OnPollEvent;
	FOnDAPollEventStatic OnPollEventStatic;

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
