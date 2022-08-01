// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientDataAndServerSessions.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/Engine.h"


UClientDataAndServerSessions::UClientDataAndServerSessions()
: DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompleted))
{
	MySessionName = FName("My Session");
	iPlayerID = 0;
	iMaxSessionPlayers = 2;
}

void UClientDataAndServerSessions::Init()
{
	//Access OnlineSubsystem
	if (IOnlineSubsystem* SubSystem = IOnlineSubsystem::Get())
	{
		//Access Session Interface
		SessionInterface = SubSystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			//Bind Delegates Here
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
				this, &UClientDataAndServerSessions::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
				this, &UClientDataAndServerSessions::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
				this, &UClientDataAndServerSessions::OnJoinSessionComplete);
			//SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
			//	this, &UClientDataAndServerSessions::OnDestroySessionComplete);

			GetEngine()->OnNetworkFailure().AddUObject(this, &UClientDataAndServerSessions::HandleNetworkFailure);
		}
	}
}

void UClientDataAndServerSessions::CreateServer(FCreateServerInfo ServerInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("Creating Server"));

	FOnlineSessionSettings SessionSettings;

	//Check if we are using the NULL Subsystem for testing purposes
	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
	{
		SessionSettings.bIsLANMatch = false;
	}
	else
	{
		SessionSettings.bIsLANMatch = true;
	}

	//Set Keys For the Server name
	SessionSettings.Set(FName("SERVER_NAME_KEY"), ServerInfo.ServerName,
	                    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	//Set all Settings needed for the session
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = iMaxSessionPlayers;

	SessionInterface->CreateSession(iPlayerID, MySessionName, SessionSettings);
}

void UClientDataAndServerSessions::OnCreateSessionComplete(FName SessionName, bool Succeeded)
{
	UE_LOG(LogTemp, Warning, TEXT("OnCreateSessionSucceeded, Succeeded: %d"), Succeeded);

	//Check if the session creation is Succeeded
	if (Succeeded)
	{
		GetWorld()->ServerTravel("/Game/3-Scenes/Arena?listen");
	}
}

void UClientDataAndServerSessions::FindServers()
{
	//Check if we are searching for servers
	bSearchingForServers.Broadcast(true);

	UE_LOG(LogTemp, Warning, TEXT("Looking For Servers"));

	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	//Check if we are using Null subsystem for testing purposes
	if (IOnlineSubsystem::Get()->GetSubsystemName() != "NULL")
	{
		SessionSearch->bIsLanQuery = false;
	}
	else
	{
		SessionSearch->bIsLanQuery = true;
	}

	//Set the max Search Results for the Steam App Space War sessions
	SessionSearch->MaxSearchResults = 20000;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(iPlayerID, SessionSearch.ToSharedRef());
}

void UClientDataAndServerSessions::OnFindSessionComplete(bool Succeeded)
{
	//Check if we are not searching for servers
	bSearchingForServers.Broadcast(false);

	UE_LOG(LogTemp, Warning, TEXT("OnFindSessionSucceeded, Succeeded: %d"), Succeeded);

	if (Succeeded)
	{
		int32 ArrayIndex = -1;

		//Get the Server info
		for (FOnlineSessionSearchResult Result : SessionSearch->SearchResults)
		{
			++ArrayIndex;

			if (!Result.IsValid())
			{
				continue;
			}

			FServerInfo Info;

			//Set default Server
			FString ServerName = "Empty Server Name";

			//Get Server Name from the creator of the session
			Result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName);

			//Apply the collected Server Info
			Info.ServerName = ServerName;
			Info.iMaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.iCurrentPlayers = Info.iMaxPlayers - Result.Session.NumOpenPublicConnections;
			//Only works if we are using Steam due to the OnlineSubsystem

			Info.SetPlayerCount();
			Info.bIsLan = Result.Session.SessionSettings.bIsLANMatch;
			Info.iPing = Result.PingInMs;
			Info.iServerArrayIndex = ArrayIndex;
			ServerListDel.Broadcast(Info);
		}

		UE_LOG(LogTemp, Warning, TEXT("Search Results, Server Count: %d"), SessionSearch->SearchResults.Num());
	}
}

void UClientDataAndServerSessions::FindMatch()
{
	int32 index = FMath::RandRange(0, SessionSearch->SearchResults.Num() - 1);

	FOnlineSessionSearchResult Result = SessionSearch->SearchResults[index];


	if (Result.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Join server at Index: %d"), index);
		SessionInterface->JoinSession(iPlayerID, MySessionName, Result); //join the server
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed To join server at Index: %d"), index);
	}
}

void UClientDataAndServerSessions::JoinServer(int32 ArrayIndex)
{
	//Grab the search result inside the array
	FOnlineSessionSearchResult Result = SessionSearch->SearchResults[ArrayIndex];

	if (Result.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Join server at Index: %d"), ArrayIndex);
		SessionInterface->JoinSession(iPlayerID, MySessionName, Result); //join the server
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed To join server at Index: %d"), ArrayIndex);
	}
}

void UClientDataAndServerSessions::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("Joining Server, SessionName: %s"), *SessionName.ToString());

	if (APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), iPlayerID))
	{
		FString JoinAddress;
		SessionInterface->GetResolvedConnectString(SessionName, JoinAddress);

		if (JoinAddress != "")
		{
			PController->ClientTravel(JoinAddress, TRAVEL_Absolute);
		}
	}
}

void UClientDataAndServerSessions::DestroySession()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
	 
	
	if (!Sessions.IsValid())
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle =
		Sessions->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!Sessions->DestroySession(MySessionName))
	{
		Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		OnDestroySessionCompleteEvent.Broadcast(false);
	}
	}
}

void UClientDataAndServerSessions::OnDestroySessionCompleted(FName SessionName, bool Successful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		
		if (Sessions)
		{
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}

		OnDestroySessionCompleteEvent.Broadcast(Successful);
	}
}

void UClientDataAndServerSessions::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainMenu", true);

}
