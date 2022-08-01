// Fill out your copyright notice in the Description page of Project Settings.


#include "GS_Arena.h"
#include "Net/UnrealNetwork.h"

AGS_Arena::AGS_Arena()
{
	SecondsInServer = 5;
	MinutesInServer = 0;
	GamePlayerOneKills = 0;
	GamePlayerTwoKills = 0;
}

void AGS_Arena::SetPlayerOneKills(int Value)
{
	GamePlayerOneKills = Value;
}

void AGS_Arena::SetHostShotsFired(int Value)
{
	iMatchHostShotsFired = Value;
}

void AGS_Arena::SetHostShotsHit(int Value)
{
	iMatchHostShotsHit = Value;
}

void AGS_Arena::SetHostShotsMissed(int Value)
{
	iMatchHostShotsMissed = Value;
}

void AGS_Arena::SetHostShardsDelivered(int Value)
{
	iMatchHostShardsDelivered = Value;
}

void AGS_Arena::SetHostOvercharges(int Value)
{
    iMatchHostOvercharges = Value;
}

void AGS_Arena::GetServerTimerMinutes(int MinutesInGameMode)
{
	MinutesInServer = MinutesInGameMode;
}

void AGS_Arena::GetServerTimerSeconds(int SecondsInGameMode)
{
	SecondsInServer = SecondsInGameMode;
}

void AGS_Arena::SetPlayerTwoKills(int Value)
{
	GamePlayerTwoKills = Value;
}

void AGS_Arena::SetClientShotsFired(int Value)
{
	iMatchClientShotsFired = Value;
}

void AGS_Arena::SetClientShotsHit(int Value)
{
	iMatchClientShotsHit = Value;
}

void AGS_Arena::SetClientShotsMissed(int Value)
{
	iMatchClientShotsMissed = Value;
}

void AGS_Arena::SetClientShardsDelivered(int Value)
{
	iMatchClientShardsDelivered = Value;
}

void AGS_Arena::SetClientOvercharges(int Value)
{
	iMatchClientOverchargeStages = Value;
}

void AGS_Arena::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Arena, SecondsInServer);
	DOREPLIFETIME(AGS_Arena, MinutesInServer);
	DOREPLIFETIME(AGS_Arena, GamePlayerOneKills);
    DOREPLIFETIME(AGS_Arena, GamePlayerTwoKills);
	DOREPLIFETIME(AGS_Arena, iMatchHostShotsFired);
	DOREPLIFETIME(AGS_Arena, iMatchHostShotsHit);
	DOREPLIFETIME(AGS_Arena, iMatchHostShotsMissed);
	DOREPLIFETIME(AGS_Arena, iMatchHostShardsDelivered);
	DOREPLIFETIME(AGS_Arena, iMatchHostOvercharges);
	DOREPLIFETIME(AGS_Arena, iMatchClientShotsFired);
	DOREPLIFETIME(AGS_Arena, iMatchClientShotsMissed);
	DOREPLIFETIME(AGS_Arena, iMatchClientShotsHit);
	DOREPLIFETIME(AGS_Arena, iMatchClientShardsDelivered);
	DOREPLIFETIME(AGS_Arena, iMatchClientOverchargeStages);
}

