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

/** Verb used by the request */
UENUM(BlueprintType)
enum class ERequestVerb : uint8
{
	GET,
	POST
};

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

	/** Sets AuthorizationCode from OAuth */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	void SetAuthorizationCode(const FString& InAuthorizationCode);

protected:
	/** Load save game and extract data */
	void LoadData();

	/** Save cached data or reset one if necessary */
	void SaveData();

	/** Create http request and add API meta */
	TSharedRef<IHttpRequest> CreateHttpRequest(const FString& Url, const FString& BodyContent = TEXT(""), ERequestVerb Verb = ERequestVerb::POST);

	/** Serialize json object into string */
	FString SerializeJson(const TSharedPtr<FJsonObject> DataJson) const;

protected:
	static const FString DonationAlertsApiEndpoint;

public:
	/** Get auth url to be opened in browser */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts|Controller")
	FString GetAuthUrl() const;

protected:
	/** Cached AppId */
	FString AppId;

	/** Cached AuthorizationCode */
	FString AuthorizationCode;

protected:
	/** Browser widget class to be used when no custom override is used */
	UPROPERTY()
	TSubclassOf<UUserWidget> DefaultBrowserWidgetClass;
};
