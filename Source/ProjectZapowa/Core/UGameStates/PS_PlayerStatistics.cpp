// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZapowa/Core/UGameStates/PS_PlayerStatistics.h"



APS_PlayerStatistics::APS_PlayerStatistics()
{
	iMatchKills = 0;
	iMatchShotsFired = 0;
	iMatchShotsHit = 0;
	iMatchShotsMissed = 0;
	iMatchShardsDelivered = 0;
	iMatchOverchargeStages = 0;
	
}

void APS_PlayerStatistics::AddKill()
{
	++iMatchKills;
}

void APS_PlayerStatistics::AddShotsFired()
{
	++iMatchShotsFired;
}

void APS_PlayerStatistics::AddShotsHit()
{
	++iMatchShotsHit;
}

void APS_PlayerStatistics::AddShotsMissed()
{
	++iMatchShotsMissed;
}

void APS_PlayerStatistics::AddShardsDelivered()
{
	++iMatchShardsDelivered;
}

void APS_PlayerStatistics::AddOvercharges()
{
	++iMatchOverchargeStages;
}

