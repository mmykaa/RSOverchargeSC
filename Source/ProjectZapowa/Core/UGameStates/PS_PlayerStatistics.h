// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PS_PlayerStatistics.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZAPOWA_API APS_PlayerStatistics : public APlayerState
{
	GENERATED_BODY()

	APS_PlayerStatistics();



public:
	UPROPERTY() int iMatchKills;
	UPROPERTY() int iMatchShotsFired;
	UPROPERTY() int iMatchShotsMissed;
	UPROPERTY() int iMatchShotsHit;
	UPROPERTY() int iMatchShardsDelivered;
	UPROPERTY() int iMatchOverchargeStages;
	
	UFUNCTION() void AddKill();
	UFUNCTION(BlueprintCallable) int GetKills() {return iMatchKills;}
	UFUNCTION() void AddShotsFired();
	UFUNCTION(BlueprintCallable) int GetShotsFired() {return iMatchShotsFired;}
	UFUNCTION() void AddShotsHit();
	UFUNCTION(BlueprintCallable) int GetShotsHit() {return iMatchShotsHit;}
	UFUNCTION() void AddShotsMissed();
	UFUNCTION(BlueprintCallable) int GetShotsMissed() {return iMatchShotsMissed;}
	UFUNCTION() void AddShardsDelivered();
	UFUNCTION(BlueprintCallable) int GetShardsDelivered() {return iMatchShardsDelivered;}
	UFUNCTION() void AddOvercharges();
	UFUNCTION(BlueprintCallable) int GetOvercharges() {return iMatchOverchargeStages;}

};

