// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#include "DonationAlertsSave.h"

#include "DonationAlertsDefines.h"

#include "Kismet/GameplayStatics.h"

const FString UDonationAlertsSave::SaveSlotName = "DonationAlertsSaveSlot";
const int32 UDonationAlertsSave::UserIndex = 0;

FDonationAlertsSaveData UDonationAlertsSave::Load()
{
	auto SaveInstance = Cast<UDonationAlertsSave>(UGameplayStatics::LoadGameFromSlot(UDonationAlertsSave::SaveSlotName, UDonationAlertsSave::UserIndex));
	if (!SaveInstance)
	{
		return FDonationAlertsSaveData();
	}

	return SaveInstance->SaveData;
}

void UDonationAlertsSave::Save(const FDonationAlertsSaveData& InSaveData)
{
	auto SaveInstance = Cast<UDonationAlertsSave>(UGameplayStatics::LoadGameFromSlot(UDonationAlertsSave::SaveSlotName, UDonationAlertsSave::UserIndex));
	if (!SaveInstance)
	{
		SaveInstance = Cast<UDonationAlertsSave>(UGameplayStatics::CreateSaveGameObject(UDonationAlertsSave::StaticClass()));
	}

	SaveInstance->SaveData = InSaveData;

	UGameplayStatics::SaveGameToSlot(SaveInstance, UDonationAlertsSave::SaveSlotName, 0);
}
