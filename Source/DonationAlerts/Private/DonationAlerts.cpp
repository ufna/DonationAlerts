// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "DonationAlerts.h"

#include "DonationAlertsDefines.h"
#include "DonationAlertsSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "Engine/World.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "FDonationAlertsModule"

void FDonationAlertsModule::StartupModule()
{
	DonationAlertsSettings = NewObject<UDonationAlertsSettings>(GetTransientPackage(), "DonationAlertsSettings", RF_Standalone);
	DonationAlertsSettings->AddToRoot();

	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "DonationAlerts",
			LOCTEXT("RuntimeSettingsName", "DonationAlerts"),
			LOCTEXT("RuntimeSettingsDescription", "Configure DonationAlerts"),
			DonationAlertsSettings);
	}

	UE_LOG(LogDonationAlerts, Log, TEXT("%s: DonationAlerts module started"), *VA_FUNC_LINE);
}

void FDonationAlertsModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "DonationAlerts");
	}

	if (!GExitPurge)
	{
		// If we're in exit purge, this object has already been destroyed
		DonationAlertsSettings->RemoveFromRoot();
	}
	else
	{
		DonationAlertsSettings = nullptr;
	}
}

UDonationAlertsSettings* FDonationAlertsModule::GetSettings() const
{
	check(DonationAlertsSettings);
	return DonationAlertsSettings;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDonationAlertsModule, DonationAlerts)

DEFINE_LOG_CATEGORY(LogDonationAlerts);
