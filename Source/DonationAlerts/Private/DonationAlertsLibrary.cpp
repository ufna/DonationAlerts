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

UDonationAlertsController* UDonationAlertsLibrary::GetDonationAlertsController(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return FDonationAlertsModule::Get().GetController(World);
	}

	return nullptr;
}

UDonationAlertsSettings* UDonationAlertsLibrary::GetDonationAlertsSettings(UObject* WorldContextObject)
{
	return FDonationAlertsModule::Get().GetSettings();
}