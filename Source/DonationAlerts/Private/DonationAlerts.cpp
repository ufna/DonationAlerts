// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "DonationAlerts.h"

#include "DonationAlertsController.h"
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

	FWorldDelegates::OnWorldCleanup.AddLambda([this](UWorld* World, bool bSessionEnded, bool bCleanupResources) {
		DonationAlertsControllers.Remove(World);

		UE_LOG(LogDonationAlerts, Log, TEXT("%s: DonationAlerts Controller is removed for: %s"), *VA_FUNC_LINE, *World->GetName());
	});

	FWorldDelegates::OnPostWorldInitialization.AddLambda([this](UWorld* World, const UWorld::InitializationValues IVS) {
		auto PluginController = NewObject<UDonationAlertsController>(GetTransientPackage());
		PluginController->SetFlags(RF_Standalone);
		PluginController->AddToRoot();

		// Initialize controller with default settings
		PluginController->Initialize(DonationAlertsSettings->AppId);

		DonationAlertsControllers.Add(World, PluginController);

		UE_LOG(LogDonationAlerts, Log, TEXT("%s: DonationAlerts Controller is created for: %s"), *VA_FUNC_LINE, *World->GetName());
	});

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

		for (auto PluginController : DonationAlertsControllers)
		{
			PluginController.Value->RemoveFromRoot();
		}
	}
	else
	{
		DonationAlertsSettings = nullptr;
	}

	DonationAlertsControllers.Empty();
}

UDonationAlertsSettings* FDonationAlertsModule::GetSettings() const
{
	check(DonationAlertsSettings);
	return DonationAlertsSettings;
}

UDonationAlertsController* FDonationAlertsModule::GetController(UWorld* World) const
{
	return DonationAlertsControllers.FindChecked(World);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDonationAlertsModule, DonationAlerts)

DEFINE_LOG_CATEGORY(LogDonationAlerts);
