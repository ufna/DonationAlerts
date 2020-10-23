// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"

#include "DonationAlertsSettings.generated.h"

UCLASS(config = Engine, defaultconfig)
class DONATIONALERTS_API UDonationAlertsSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Application ID */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DonationAlerts Settings")
	FString AppId;

	/** AuthCode to AccessToken exchange url https://www.donationalerts.com/apidoc#authorization__authorization_code__getting_access_token */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DonationAlerts Settings")
	FString AuthTokenExchangeURI;

	/** Custom class to handle OAuth */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "DonationAlerts Settings")
	TSubclassOf<UUserWidget> OverrideBrowserWidgetClass;
};
