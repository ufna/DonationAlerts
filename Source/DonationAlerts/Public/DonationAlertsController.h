// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Blueprint/UserWidget.h"
#include "Delegates/DelegateCombinations.h"
#include "Http.h"
#include "Tickable.h"

#include "DonationAlertsController.generated.h"

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

UCLASS()
class DONATIONALERTS_API UDonationAlertsController : public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

protected:
	// FTickableGameObject begin
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual TStatId GetStatId() const override { return TStatId(); }
	// FTickableGameObject end

public:
	/** Initialize controller with provided data (used to override project settings) */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	void Initialize(const FString& InAppId);

	/** Opens browser to authenicate user using OAuth */
	void OpenAuthConsole(UUserWidget*& BrowserWidget);

	/** Exchange AuthorizationCode to OAuth AccessToken */
	void FetchAccessToken(const FString& InAuthorizationCode, const FOnFetchTokenSuccess& SuccessCallback, const FOnRequestError& ErrorCallback);

	/** Sets AuthorizationCode from DA */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	void SetAuthorizationCode(const FString& InAuthorizationCode);

	/** Sets AccessToken from OAuth */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	void SetAuthToken(const FDonationAlertsAuthToken& InAuthToken);

	/** custom_alert API caller */
	void SendCustomAlert(const FString& ExternalId, const FString& Header = TEXT(""), const FString& Message = TEXT(""), const FString& ImageUrl = TEXT(""), const FString& SoundUrl = TEXT(""), const FOnRequestError& ErrorCallback = FOnRequestError());

protected:
	void FetchAccessToken_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnFetchTokenSuccess SuccessCallback, FOnRequestError ErrorCallback);
	void SendCustomAlert_HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, FOnRequestError ErrorCallback);
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
	static const FString DonationAlertsApiEndpoint;

public:
	/** Get auth url to be opened in browser */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	FString GetAuthUrl() const;

	/** Get cached auth token */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	FDonationAlertsAuthToken GetAuthToken() const;

protected:
	/** Cached AppId */
	FString AppId;

	/** Cached AuthorizationCode */
	FString AuthorizationCode;

	/** Cached AuthToken */
	FDonationAlertsAuthToken AuthToken;

protected:
	/** Browser widget class to be used when no custom override is used */
	UPROPERTY()
	TSubclassOf<UUserWidget> DefaultBrowserWidgetClass;
};
