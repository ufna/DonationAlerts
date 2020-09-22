// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "DonationAlertsLibrary.h"

#include "DonationAlerts.h"
#include "DonationAlertsSettings.h"

#include "Engine/Engine.h"

UDonationAlertsLibrary::UDonationAlertsLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UDonationAlertsSettings* UDonationAlertsLibrary::GetDonationAlertsSettings(UObject* WorldContextObject)
{
	return FDonationAlertsModule::Get().GetSettings();
}

int64 UDonationAlertsLibrary::DateTimeToUnixTime(const FDateTime& DateTime)
{
	return DateTime.ToUnixTimestamp();
}
