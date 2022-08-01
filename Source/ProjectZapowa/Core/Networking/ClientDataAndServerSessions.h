// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ClientDataAndServerSessions.generated.h"


USTRUCT(BlueprintType)
struct FCreateServerInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString ServerName;

	UPROPERTY(BlueprintReadWrite)
	bool bIsLan;
};

USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_BODY()

public:
	//Sessions
	UPROPERTY(BlueprintReadOnly)
	FString ServerName;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerCountStr;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLan;

	UPROPERTY(BlueprintReadOnly)
	int32 iPing;

	UPROPERTY(BlueprintReadOnly)
	int32 iServerArrayIndex;

	int32 iCurrentPlayers;

	int32 iMaxPlayers;

	void SetPlayerCount()
	{
		PlayerCountStr = FString(FString::FromInt(iCurrentPlayers) + "/" + FString::FromInt(iMaxPlayers));
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerDel, FServerInfo, ServerListDel);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerSearchingDel, bool, bSearchingForServers);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnDestroySessionComplete, bool, Successful);

UCLASS()
class PROJECTZAPOWA_API UClientDataAndServerSessions : public UGameInstance
{
	GENERATED_BODY()

public:
	UClientDataAndServerSessions();


protected:
	FName MySessionName;

	UPROPERTY(BlueprintAssignable)
	FServerDel ServerListDel;

	UPROPERTY(BlueprintAssignable)
	FServerSearchingDel bSearchingForServers;


	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	int iPlayerID;

	int iMaxSessionPlayers;

	virtual void Init() override;

	virtual void OnCreateSessionComplete(FName SessionName, bool Succeeded);

	virtual void OnFindSessionComplete(bool Succeeded);

	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	virtual void OnDestroySessionCompleted(FName SessionName, bool Succeeded);
	
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BlueprintCallable)
	void CreateServer(FCreateServerInfo ServerInfo);

	UFUNCTION(BlueprintCallable)
	void FindServers();

	UFUNCTION(BlueprintCallable)
	void FindMatch();

	UFUNCTION(BlueprintCallable)
	void JoinServer(int32 ArrayIndex);

public:
	UFUNCTION(BlueprintCallable)
	void DestroySession();
	FCSOnDestroySessionComplete OnDestroySessionCompleteEvent;
	void HandleNetworkFailure(UWorld * World, UNetDriver * NetDriver, ENetworkFailure::Type FailureType, const FString & ErrorString);
};
