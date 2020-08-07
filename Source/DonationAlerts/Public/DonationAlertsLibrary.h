// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/DateTime.h"

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

	/** Helper function to get Unix Timestamp from FDateTime param */
	UFUNCTION(BlueprintPure, Category = "DonationAlerts")
	static int64 DateTimeToUnixTime(const FDateTime& DateTime);
};
