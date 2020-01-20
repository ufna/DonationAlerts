// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "GameFramework/SaveGame.h"

#include "DonationAlertsDefines.h"

#include "DonationAlertsSave.generated.h"

USTRUCT(Blueprintable)
struct DONATIONALERTS_API FDonationAlertsSaveData
{
	GENERATED_USTRUCT_BODY()

	/** Last used access token to cache auth data */
	UPROPERTY()
	FString AccessToken;

	FDonationAlertsSaveData(){};

	FDonationAlertsSaveData(FString InAccessToken)
		: AccessToken(InAccessToken){};
};

UCLASS()
class DONATIONALERTS_API UDonationAlertsSave : public USaveGame
{
	GENERATED_BODY()

public:
	static FDonationAlertsSaveData Load();
	static void Save(const FDonationAlertsSaveData& InSaveData);

public:
	static const FString SaveSlotName;

	/** User index (always 0) */
	static const int32 UserIndex;

protected:
	UPROPERTY()
	FDonationAlertsSaveData SaveData;
};
