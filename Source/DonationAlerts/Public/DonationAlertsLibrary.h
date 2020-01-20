// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "DonationAlertsController.h"

#include "DonationAlertsLibrary.generated.h"

class UDonationAlertsSettings;

UCLASS()
class DONATIONALERTS_API UDonationAlertsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	/** Direct access to DonationAlerts controller */
	UFUNCTION(BlueprintPure, Category = "DonationAlerts", meta = (WorldContext = "WorldContextObject"))
	static UDonationAlertsController* GetDonationAlertsController(UObject* WorldContextObject);

	/** Direct access to DonationAlerts settings */
	UFUNCTION(BlueprintPure, Category = "DonationAlerts", meta = (WorldContext = "WorldContextObject"))
	static UDonationAlertsSettings* GetDonationAlertsSettings(UObject* WorldContextObject);

	/** User will be prompted by the service to authorize or deny the application access to their account */
	UFUNCTION(BlueprintCallable, Category = "DonationAlerts", meta = (WorldContext = "WorldContextObject"))
	static void AuthenicateUser(UObject* WorldContextObject, UUserWidget*& BrowserWidget);
};
