// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GS_Arena.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZAPOWA_API AGS_Arena : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGS_Arena();


#pragma region Replicate MatchInfo
	
	/** Minutes In Server */
	UPROPERTY(Replicated, BlueprintReadOnly) int MinutesInServer;

	/** Seconds In Server  */
	UPROPERTY(Replicated, BlueprintReadOnly) int SecondsInServer;
	
	/** Host */
    UPROPERTY(Replicated, BlueprintReadOnly) int GamePlayerOneKills;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchHostShotsFired;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchHostShotsMissed;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchHostShotsHit;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchHostShardsDelivered;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchHostOvercharges;
	
	UFUNCTION() void SetPlayerOneKills(int Value);
	UFUNCTION() void SetHostShotsFired(int Value); 
	UFUNCTION() void SetHostShotsHit(int Value);
	UFUNCTION() void SetHostShotsMissed(int Value); 
	UFUNCTION() void SetHostShardsDelivered(int Value);
	UFUNCTION() void SetHostOvercharges(int Value);

	
    /** Client */
    UPROPERTY(Replicated, BlueprintReadOnly) int GamePlayerTwoKills;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchClientShotsFired;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchClientShotsMissed;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchClientShotsHit;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchClientShardsDelivered;
	UPROPERTY(Replicated,BlueprintReadOnly) int iMatchClientOverchargeStages;
	
    UFUNCTION() void SetPlayerTwoKills(int Value);
	UFUNCTION() void SetClientShotsFired(int Value); 
	UFUNCTION() void SetClientShotsHit(int Value);
	UFUNCTION() void SetClientShotsMissed(int Value); 
	UFUNCTION() void SetClientShardsDelivered(int Value);
	UFUNCTION() void SetClientOvercharges(int Value);

	///////////////////////////////////////////////////////////////////////////////////////

	/** Get Server Minutes */
	UFUNCTION()	void GetServerTimerMinutes(int MinutesInGameMode);

	/** Get Server Seconds */
	UFUNCTION()	void GetServerTimerSeconds(int SecondsInGameMode);

#pragma  endregion Replicate MatchInfo

	
};
