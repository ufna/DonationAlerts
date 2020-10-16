// Copyright 2019 My.com B.V. All Rights Reserved.
// @author Vladimir Alyamkin <ufna@ufna.ru>

#pragma once

#include "CoreMinimal.h"

#include "DonationAlertsTypes.generated.h"

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsAuthToken
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "AuthToken")
	FString access_token;

	UPROPERTY(BlueprintReadWrite, Category = "AuthToken")
	int64 expires_in;

	UPROPERTY(BlueprintReadWrite, Category = "AuthToken")
	FString refresh_token;

public:
	FDonationAlertsAuthToken()
		: expires_in(0){};

	bool IsValid() const { return !access_token.IsEmpty() && !refresh_token.IsEmpty(); }
};

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsUserProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "UserProfile")
	int32 id;

	UPROPERTY(BlueprintReadWrite, Category = "UserProfile")
	FString socket_connection_token;

public:
	FDonationAlertsUserProfile()
		: id(0){};
};

/** Verb used by the request */
UENUM(BlueprintType)
enum class ERequestVerb : uint8
{
	GET,
	POST
};

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsDonationEvent
{
	GENERATED_BODY()

	/** The unique alert identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	int32 id;

	/** Type of the alert. Always donation in this case */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	FString name;

	/** The name of the user who sent the donation and the alert */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	FString username;

	/** The message type. The possible values are text for a text messages and audio for an audio messages */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	FString message_type;

	/** The message sent along with the donation and the alert */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	FString message;

	/** The donation amount */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	int32 amount;

	/** The currency code (ISO 4217 formatted) */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	FString currency;

	/** A flag indicating whether the alert was shown in the streamer's widget */
	UPROPERTY(BlueprintReadWrite, Category = "Donation Event")
	bool is_shown;

public:
	FDonationAlertsDonationEvent()
		: id(0)
		, amount(0)
		, is_shown(false){};
};

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsGoalEvent
{
	GENERATED_BODY()

	/** The unique event identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	int32 id;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	bool is_active;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	FString title;

	/** The currency code (ISO 4217 formatted) */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	FString currency;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	int32 start_amount;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	int32 raised_amount;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Goal Event")
	int32 goal_amount;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "GoalEvent")
	FString reason;

public:
	FDonationAlertsGoalEvent()
		: id(0)
		, is_active(false)
		, start_amount(0)
		, raised_amount(0)
		, goal_amount(0){};
};

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsPollOption
{
	GENERATED_BODY()

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Option")
	int32 id;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Option")
	FString title;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Option")
	int32 amount_value;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Option")
	int32 amount_percent;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Option")
	bool is_winner;

public:
	FDonationAlertsPollOption()
		: id(0)
		, amount_value(0)
		, amount_percent(0)
		, is_winner(false){};
};

USTRUCT(BlueprintType)
struct DONATIONALERTS_API FDonationAlertsPollEvent
{
	GENERATED_BODY()

	/** The unique event identifier */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Event")
	int32 id;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Event")
	bool is_active;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Event")
	FString title;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Event")
	FString type;

	/** */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Event")
	TArray<FDonationAlertsPollOption> options;

	/**  */
	UPROPERTY(BlueprintReadWrite, Category = "Poll Event")
	FString reason;

public:
	FDonationAlertsPollEvent()
		: id(0)
		, is_active(false){};
};
