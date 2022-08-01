// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Arena.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTZAPOWA_API AGM_Arena : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGM_Arena();

	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;

	AActor* Anouncer;

UFUNCTION() void PlaySound(int SoundToPlayInIndex);

#pragma region ArenaGeneration

	/**  */
	UPROPERTY(EditAnywhere, Category = "Arena Layout") TArray<TSubclassOf<AActor>> aArenaLayoutTiles;

	/**  */
	UPROPERTY(EditAnywhere, Category = "Arena Layout") TArray<FVector> aArenaTilePosition;

	/**  */
	UPROPERTY(EditAnywhere, Category = "Arena Layout") TArray<AActor*> aArenaLayoutTilesSpawned;
	
	/**  */
	UPROPERTY() bool isArenaLayoutGenerated;

	//////////////////////////////////////////////////////////////////////////

	/**  */
	UFUNCTION() void CheckIfAllArenaTilesExists();

	/**  */
	UFUNCTION() void CheckIfArenaLayoutIsCreated();

	/**  */
	UFUNCTION() void GenerateArenaLayout();

	/**  */
	UFUNCTION(BlueprintImplementableEvent) void StartSkyChanges();

#pragma endregion ArenaGeneration

#pragma region MatchTimer

	FTimerHandle InGameMatchTimer;

	/**  */
	UPROPERTY(BlueprintReadOnly) int iMatchTimeMinutes;

	/**  */
	UPROPERTY(BlueprintReadOnly) int iMatchTimeSeconds;
	
	/**  */
	UPROPERTY(BlueprintReadOnly) int iMinutes;

	/**  */
	UPROPERTY(BlueprintReadOnly) int iSeconds;

	UPROPERTY() bool bIsWaitingToReturnTOMainMenu;

	//////////////////////////////////////////////////////////////////////////

	/**  */
	UFUNCTION() void DecrementTime();

	/**  */
	UFUNCTION() void SendTimerMinutesToGameState() const;

	/**  */
	UFUNCTION() void SendTimerSecondsToGameState() const;

#pragma endregion MatchTimer

#pragma region PlayerRespawn

	/**  */
	UPROPERTY(EditAnywhere, Category = "RespawnBeacons") TArray<FVector> aRespawnBeacons;
	
	/**  */
	UPROPERTY() AActor* PlayerToRespawn;

	/**  */
	UPROPERTY() FVector vKillerLocation;

	/**  */
	UPROPERTY() int iLongestIndex;

	/**  */
	UPROPERTY() int iFirstCheckLongestIndex;

	/**  */
	UPROPERTY() int iSecondCheckLongestIndex;

	/**  */
	UPROPERTY() bool bFinishedFirstCheck;

	/**  */
	UPROPERTY() float fFirstDistanceToKeep;

	/**  */
	UPROPERTY() float fSecondDistanceToKeep;

	/**  */
	UPROPERTY() int iLongestDistanceIndex;

	UPROPERTY() FVector vWorldSpawnLocation;

	
	//////////////////////////////////////////////////////////////////////////

	
	/**  */
	UFUNCTION() void RunRespawnTimer(const AActor* Killer, AActor* Player,  AController* Controller);

	/**  */
	UFUNCTION() void GetDistanceToRespawnBeacons();

	/**  */
	UFUNCTION() void ChooseRespawnPoint(int iRespawnBeaconChosen);

	
	UFUNCTION() FTransform GetSpawnTransform();


#pragma endregion PlayerRespawn

#pragma region Scores

	/** Stores the Player One Index */
	UPROPERTY()	int PlayerOneIndex;

	/** Stores the player two index */
	UPROPERTY()	int PlayerTwoIndex;

	//////////////////////////////////////////////////////////////////

	/** Updates the score for each player */
	UFUNCTION() void UpdatePlayerScores(AActor* Killer);

	/** Updates the match score */
	UFUNCTION() void UpdateMatchScores();
	
	/** Gets the final score results and Chooses the winner */
	UFUNCTION() void ChooseWinner();

#pragma endregion Scores

#pragma region SkyDrops

	/** Stores the int value that is associated with the Escape Pod type */
	UPROPERTY() int iEscapePodType;

	/** Stores the int value that is associated with the Altar Pod type */
	UPROPERTY() int iAltarPodType;

	/** Current Arena Tile To Spawn Drop */
	UPROPERTY() int iCurrentArenaToSpawnDropIndex;

	/** Used to create the Spawning Pattern */
	UPROPERTY()	int iAmountOfEscapePodsSpawned;

	/** Used to create the Spawning Pattern */
	UPROPERTY()	int iAmountOfAltarPodsSpawned;

	/** Timer that Handles the Spawning of the Drops */
	UPROPERTY() FTimerHandle fTHSpawnDrops;

	/** Max amount of drops to spawn */
	UPROPERTY()	int iMaxAmountOfDropsToSpawn;

	/** Current amount of drops Spawned */
	UPROPERTY()	int iCurrentAmountOfDropsSpawned;

	/** Current amount of drops Spawned */
	UPROPERTY()	float fEscapePodsSpawnRate;

	/** Minutes To Start Spawning EscapePodes */
	UPROPERTY()	int iMinutesToStartSpawnEscapePods;

	/** Minutes To Start Stage Three */
	UPROPERTY()	int iMinutesToStartStageThree;

	/** Minutes To Start Stage Four */
	UPROPERTY()	int iMinutesToStartStageFour;

	UPROPERTY() TArray<AActor*> EscapePodsFound;

	////////////////////////////////////////////////////////////////////////////////

	
	/** Chooses in which tile to spawn the drop */
	UFUNCTION()	void ChooseTileToSpawnDrop(int Index, int Type);

	/** Handles the Drop Stage 1 Logic */
	UFUNCTION() void SkyDropsStageOne();
	
	/** Handles the Drop Stage 2 Logic */
	UFUNCTION() void SkyDropsStageTwo();

	/** Handles the Drop Stage 3 Logic */
	UFUNCTION() void SkyDropsStageThree();

	/** Handles the Drop Stage 4 Logic */
	UFUNCTION() void SkyDropsStageFour();

	/** Resets Pods */
	UFUNCTION() void ResetPods();


#pragma endregion SkyDrops

#pragma region StartMatch
	UPROPERTY() FTimerHandle BeforeMatchTimer;
	UPROPERTY() FTimerHandle ScoutPhaseTimer;
	UPROPERTY() FTimerHandle PostMatchTimer;
	UPROPERTY() TArray<APlayerController*>PlayersInLobby;
	UPROPERTY() APlayerController* PlayerWhoJoined;
	UPROPERTY() int iMaxNumberOfPlayers;
	UPROPERTY() int iCurrentNumberOfPlayers;

	UPROPERTY() int PlayerOneStage;
	UPROPERTY() int PlayerTwoStage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray <FVector> MatchSpawnLocations;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION() void CheckIfLobbyIsFull();
	UFUNCTION() void StartCountdown();
	UFUNCTION() void DecrementCountDown();
	UFUNCTION() void MovePlayersToTheirScoutPosition();
	UFUNCTION() void StartScoutPhase();
	UFUNCTION() void DecrementScoutPhase();
	UFUNCTION() void StartMatch();
	UFUNCTION() void EndMatchTimer();
	UFUNCTION() void DecrementPostMatchTimer();

#pragma endregion StartMatch

#pragma region EndMatch

	//** Iterate through every player and return them to the main menu */
	UFUNCTION() void EndMatch();

#pragma endregion EndMatch


};

