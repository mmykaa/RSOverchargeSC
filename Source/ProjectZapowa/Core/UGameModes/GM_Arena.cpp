// Fill out your copyright notice in the Description page of Project Settings.


#include "GM_Arena.h"

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/Collision/ParticleModuleCollisionGPU.h"
#include "ProjectZapowa/Core/Arena/ArenaLayoutTile.h"
#include "ProjectZapowa/Core/Networking/ClientDataAndServerSessions.h"
#include "ProjectZapowa/Core/Player/PlayerBehaviour.h"
#include "ProjectZapowa/Core/SkyDrops/DropEscapePod.h"
#include "ProjectZapowa/Core/UGameStates/GS_Arena.h"
#include "ProjectZapowa/Core/UGameStates/PS_PlayerStatistics.h"

AGM_Arena::AGM_Arena()
{
	// Set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn>

	PlayerPawnBPClass(TEXT("/Game/1-Entities-Components/Player/BP_Player"));

	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
	PlayerOneIndex = 0;
	PlayerTwoIndex = 1;
	
	iMatchTimeMinutes = 5;
	iMatchTimeSeconds = 0;
	iMinutes = 0;
	iSeconds = 0;
	
	bIsWaitingToReturnTOMainMenu=false;

	//Skydrops Spawning
	iCurrentArenaToSpawnDropIndex = 0;
	iMaxAmountOfDropsToSpawn = 20;
	iCurrentAmountOfDropsSpawned = 0;
	iEscapePodType = 0;
	iAltarPodType = 1;

	iMinutesToStartStageThree = 3;
	iMinutesToStartStageFour =	2;

	iMaxNumberOfPlayers = 2;
	iCurrentNumberOfPlayers = 0;
}

void AGM_Arena::BeginPlay()
{
	Super::BeginPlay();

	CheckIfAllArenaTilesExists();

}

#pragma region Login

void AGM_Arena::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AController* Controller = Cast<AController>(NewPlayer))
	{
		++iCurrentNumberOfPlayers;

		PlayersInLobby.Add(NewPlayer);
		
		Cast<APlayerBehaviour>(NewPlayer->GetPawn())->DisablePlayerInput();
		
		if (iCurrentNumberOfPlayers <= iMaxNumberOfPlayers)
		{			
			CheckIfLobbyIsFull();
		}
		else if (iCurrentNumberOfPlayers > iMaxNumberOfPlayers)
		{
			Cast<APlayerBehaviour>(NewPlayer->GetPawn())->ReturnToMainMenu();
		}
	}
}

#pragma endregion Login

#pragma region ArenaGeneration

void AGM_Arena::CheckIfAllArenaTilesExists()
{
	//Check if there is any tile that is empty
	for (int32 i = 0; i < aArenaLayoutTiles.Num(); ++i)
	{
		if (aArenaLayoutTiles[i] == nullptr)
		{
			return;
		}
	}

	CheckIfArenaLayoutIsCreated();
}

void AGM_Arena::CheckIfArenaLayoutIsCreated()
{
	if (!isArenaLayoutGenerated)
	{
		GenerateArenaLayout();
	}
}

void AGM_Arena::GenerateArenaLayout()
{
	//Set Arena Tile Rotations
	FRotator SpawnRotation;
	SpawnRotation.Yaw = 0.0f;
	SpawnRotation.Pitch = 0.0f;
	SpawnRotation.Roll = 0.0f;

	//Shuffle The ArenaLayoutTiles
	int32 const LastIndex = aArenaLayoutTiles.Num() - 1;

	for (int32 i = 0; i <= LastIndex; ++i)
	{
		int32 const Index = FMath::RandRange(i, LastIndex);

		if (i != Index)
		{
			aArenaLayoutTiles.Swap(i, Index);
		}

		AActor* SpawnedTile = GetWorld()->SpawnActor<AActor>(aArenaLayoutTiles[i], aArenaTilePosition[i], SpawnRotation);

		aArenaLayoutTilesSpawned.Add(SpawnedTile);
	}
}

#pragma endregion ArenaGeneration

#pragma region UpdateTimer

void AGM_Arena::SendTimerMinutesToGameState() const
{
	AGS_Arena* ArenaGameState = GetGameState<AGS_Arena>();

	ArenaGameState->GetServerTimerMinutes(iMinutes);
}

void AGM_Arena::SendTimerSecondsToGameState() const
{
	AGS_Arena* ArenaGameState = GetGameState<AGS_Arena>();

	ArenaGameState->GetServerTimerSeconds(iSeconds);
}


#pragma endregion UpdateTimer

#pragma region Score

void AGM_Arena::UpdatePlayerScores(AActor* Killer)
{
	if (Killer->HasAuthority())
	{
		/** Update Player staetes*/
		if (!Killer) return;
		APawn* KillerPawn = Cast<APawn>(Killer);
	
		if (!KillerPawn) return;
		AController* KillerController = KillerPawn->GetController();
	
		if (!KillerController) return;
		APS_PlayerStatistics* KillerPlayerState = KillerController->GetPlayerState<APS_PlayerStatistics>();

		if (!KillerPlayerState) return;
		KillerPlayerState->AddKill();

	}
}

void AGM_Arena::UpdateMatchScores()
{
	
	//Since Player One is always the host and we only have two players the UI will be updated based on the authority
	if (!PlayersInLobby[PlayerOneIndex] || !PlayersInLobby[PlayerTwoIndex]) return;
	
	APS_PlayerStatistics* PlayerOnePlayerState = PlayersInLobby[PlayerOneIndex]->GetPlayerState<APS_PlayerStatistics>();
	APS_PlayerStatistics* PlayerTwoPlayerState = PlayersInLobby[PlayerTwoIndex]->GetPlayerState<APS_PlayerStatistics>();

	AGS_Arena* ArenaGameState = GetGameState<AGS_Arena>();
	
	if (!ArenaGameState || !PlayerOnePlayerState || !PlayerTwoPlayerState) return;

	if (PlayerOnePlayerState->GetKills() == 1 && PlayerTwoPlayerState->GetKills() == 0 || PlayerOnePlayerState->GetKills() == 0 && PlayerTwoPlayerState->GetKills() == 1 )
	{
		PlaySound(11);
	}

	if ((PlayerOnePlayerState->GetKills() - PlayerTwoPlayerState->GetKills()) == 3 || (PlayerTwoPlayerState->GetKills() - PlayerOnePlayerState->GetKills()) == 3 )
	{
		PlaySound(13);
	}

	if ((PlayerOnePlayerState->GetKills() - PlayerTwoPlayerState->GetKills()) >= 5 || (PlayerTwoPlayerState->GetKills() - PlayerOnePlayerState->GetKills()) >= 5 )
	{
		PlaySound(9);
	}
	
	ArenaGameState->SetPlayerOneKills(PlayerOnePlayerState->GetKills());
	ArenaGameState->SetPlayerTwoKills(PlayerTwoPlayerState->GetKills());
}

void AGM_Arena::ChooseWinner()
{
	
	if (!PlayersInLobby[PlayerOneIndex] || !PlayersInLobby[PlayerTwoIndex]) return;
	
	APS_PlayerStatistics* PlayerOnePlayerState = PlayersInLobby[PlayerOneIndex]->GetPlayerState<APS_PlayerStatistics>();
	APS_PlayerStatistics* PlayerTwoPlayerState = PlayersInLobby[PlayerTwoIndex]->GetPlayerState<APS_PlayerStatistics>();

	AGS_Arena* ArenaGameState = GetGameState<AGS_Arena>();
	if (!ArenaGameState || !PlayerOnePlayerState || !PlayerTwoPlayerState) return;

	ArenaGameState->SetPlayerOneKills(PlayerOnePlayerState->GetKills());
	ArenaGameState->SetHostShotsFired(PlayerOnePlayerState->GetShotsFired()); 
	ArenaGameState->SetHostShotsHit(PlayerOnePlayerState->GetShotsHit());
	ArenaGameState->SetHostShotsMissed(PlayerOnePlayerState->GetShotsMissed()); 
	ArenaGameState->SetHostShardsDelivered(PlayerOnePlayerState->GetShardsDelivered());
	ArenaGameState->SetHostOvercharges(PlayerOnePlayerState->GetOvercharges());
	
	ArenaGameState->SetPlayerTwoKills(PlayerTwoPlayerState->GetKills());
	ArenaGameState->SetClientShotsFired(PlayerTwoPlayerState->GetShotsFired()); 
	ArenaGameState->SetClientShotsHit(PlayerTwoPlayerState->GetShotsHit());
	ArenaGameState->SetClientShotsMissed(PlayerTwoPlayerState->GetShotsMissed()); 
	ArenaGameState->SetClientShardsDelivered(PlayerTwoPlayerState->GetShardsDelivered());
	ArenaGameState->SetClientOvercharges(PlayerTwoPlayerState->GetOvercharges());
	
	int PlayerOneKills = PlayerOnePlayerState->GetKills();
	int PlayerTwoKills = PlayerTwoPlayerState->GetKills();

	
	if (PlayerOneKills > PlayerTwoKills)
	{
		/** Player One Wins */
		Cast<APlayerBehaviour>(PlayersInLobby[PlayerOneIndex]->GetPawn())->AddWinWidgetToViewport();
		Cast<APlayerBehaviour>(PlayersInLobby[PlayerTwoIndex]->GetPawn())->AddLoseWidgetToViewport();
	}
	if (PlayerOneKills < PlayerTwoKills)
	{
		/** Player Two Wins */
		Cast<APlayerBehaviour>(PlayersInLobby[PlayerOneIndex]->GetPawn())->AddLoseWidgetToViewport();
		Cast<APlayerBehaviour>(PlayersInLobby[PlayerTwoIndex]->GetPawn())->AddWinWidgetToViewport();
	}
	if (PlayerOneKills == PlayerTwoKills)
	{
		/** Tie */
		Cast<APlayerBehaviour>(PlayersInLobby[PlayerOneIndex]->GetPawn())->AddTieWidgetToViewport();
		Cast<APlayerBehaviour>(PlayersInLobby[PlayerTwoIndex]->GetPawn())->AddTieWidgetToViewport();
	}
}

#pragma endregion Score

#pragma region RespawnPlayer

void AGM_Arena::RunRespawnTimer(const AActor* Killer, AActor* Player, AController* Controller)
{
	if (!Controller) return;
	if (!Controller->GetPawn()) return;

	PlayerToRespawn = Player;

	//Check for the enemy position who killed the player
	vKillerLocation = Killer->GetActorLocation();
	
	APawn* Pawn = Controller->GetPawn();
	Pawn->Reset();
	Pawn->Destroy();
	
	GetDistanceToRespawnBeacons();
	RestartPlayerAtTransform(Controller, GetSpawnTransform());
	
	//if (Controller->GetPawn())
	//	Cast<APlayerBehaviour>(Controller->GetPawn())->HideFPModel();
	
}

void AGM_Arena::GetDistanceToRespawnBeacons()
{
	float fLongestDistance = 0.0f;
	int LongestDistanceIndex = 0;
	
	for (int i = 0; i < aRespawnBeacons.Num() - 1; ++i)
	{
		float const fDistanceBeingChecked = sqrt((pow(vKillerLocation.X - aRespawnBeacons[i].X, 2))
			+ (pow(vKillerLocation.Y - aRespawnBeacons[i].Y, 2))           //                          ________________________
			+ (pow(vKillerLocation.Z - aRespawnBeacons[i].Z, 2)));        //fDistanceBeingChecked =  _/(x-x)^2+(y-y)^2+(z-z)^2

		int const iDistanceBeingCheckedIndex = i;

		if (fDistanceBeingChecked > fLongestDistance)
		{
			fLongestDistance = fDistanceBeingChecked;
			LongestDistanceIndex = iDistanceBeingCheckedIndex;
		}
	}

	ChooseRespawnPoint(LongestDistanceIndex);
}

void AGM_Arena::ChooseRespawnPoint(int iRespawnBeaconChosen)
{
	//Get the aRespawnBeacon index to respawn the player
	FVector vStartCheckPos = aRespawnBeacons[iRespawnBeaconChosen] + FVector(0.0f, 0.0f, 10000.0f);

	FVector vEndCheckPos = aRespawnBeacons[iRespawnBeaconChosen];
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	GetWorld()->LineTraceSingleByChannel(OutHit, vStartCheckPos, vEndCheckPos, ECC_Visibility, CollisionParams);

	/** A small adjustment to the player spawn location so he can always spawn above something */
	vWorldSpawnLocation = OutHit.Location + FVector(0.0f, 0.0f, 200.0f);
}

FTransform AGM_Arena::GetSpawnTransform()
{
	FRotator rRotationToCenterOfTheArena = FRotator::ZeroRotator;
	FTransform tSpawnTransform = UKismetMathLibrary::MakeTransform(vWorldSpawnLocation, rRotationToCenterOfTheArena, FVector(1.0f,1.0f,1.0f));;
	return 	tSpawnTransform;
}

#pragma endregion RespawnPlayer

#pragma region SkyDrops

void AGM_Arena::SkyDropsStageOne()
{
	//After the first blood, a drop should spawn in front of the player
	//Two Drops should spawn in random tiles
}

void AGM_Arena::SkyDropsStageTwo()
{
	if (iCurrentArenaToSpawnDropIndex >= aArenaLayoutTilesSpawned.Num()-1)
	{
		iCurrentArenaToSpawnDropIndex = 0;
	}
	
	if (iCurrentAmountOfDropsSpawned >= iMaxAmountOfDropsToSpawn)
	{
		GetWorld()->GetTimerManager().ClearTimer(fTHSpawnDrops);
	}
	
	ChooseTileToSpawnDrop(iCurrentArenaToSpawnDropIndex, iEscapePodType);

	++iCurrentAmountOfDropsSpawned;
	++iCurrentArenaToSpawnDropIndex;
}

void AGM_Arena::SkyDropsStageThree()
{
	//UE_LOG(LogTemp, Warning, TEXT("SPAWNING TEMPLE"));

	//Altar fall from the sky to the Temple tile
	for (int32 i = 0; i < aArenaLayoutTilesSpawned.Num(); i++)
	{
		if (Cast<AArenaLayoutTile>(aArenaLayoutTilesSpawned[i])->GetTileName() == "Temple")
		{
			ChooseTileToSpawnDrop(i, iAltarPodType);
		}
	}
}

void AGM_Arena::SkyDropsStageFour()
{
	//UE_LOG(LogTemp, Warning, TEXT("SPAWNING ROUNDABOUT"));
	
	//Altar fall from the sky to the Roundabout tile
	for (int32 i = 0; i < aArenaLayoutTilesSpawned.Num(); i++)
	{
		if (Cast<AArenaLayoutTile>(aArenaLayoutTilesSpawned[i])->GetTileName() == "Roundabout")
		{
			ChooseTileToSpawnDrop(i, iAltarPodType);
		}
	}
}

void AGM_Arena::ResetPods()
{
	UClass * EscapePodsClass = ADropEscapePod::StaticClass();
	for (TActorIterator<AActor> Actor (GetWorld(),EscapePodsClass); Actor; ++Actor )
	{
		EscapePodsFound.Add(Cast<ADropEscapePod>(*Actor));
	}

	for (int i = 0; i < EscapePodsFound.Num(); ++i)
	{
		Cast<ADropEscapePod>(EscapePodsFound[i])->ResetPod();
	}
}

void AGM_Arena::ChooseTileToSpawnDrop(int Index, int Type)
{
	AArenaLayoutTile* ArenaToSpawnDrop = Cast<AArenaLayoutTile>(aArenaLayoutTilesSpawned[Index]);

	if (ArenaToSpawnDrop != nullptr)
	{
		// We need to check if there are any slots available to spawn a drop in the desired Arena Tile
		// We also have to check the type of the drop, since the altars only spawn 2 times and in different
		// tiles, as so, our main problem is with EscapePods, to prevent this we check if there are any available slots
		// and if so, we spawn the drop, if don't, we try to spawn in it the next arena tile in the array
		
		if(Type == iEscapePodType)
		{
			if (ArenaToSpawnDrop->CheckForSlots())
			{
				ArenaToSpawnDrop->CheckIfDropsExists();
				ArenaToSpawnDrop->ChooseDrop(Type);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SKIPPED: %s"),*ArenaToSpawnDrop->GetName());
			}
		}

		if(Type == iAltarPodType)
		{
			
			ArenaToSpawnDrop->CheckIfDropsExists();
			ArenaToSpawnDrop->ChooseDrop(Type);
		}
	}
}

#pragma endregion SkyDrops

#pragma region MatchTimers

void AGM_Arena::CheckIfLobbyIsFull()
{
	if (iCurrentNumberOfPlayers == iMaxNumberOfPlayers)
	{
		StartCountdown();
		StartSkyChanges();
	}
}

void AGM_Arena::StartCountdown()
{
	iMinutes = 0;
	iSeconds = 15;
	GetWorldTimerManager().SetTimer(BeforeMatchTimer, this, &AGM_Arena::DecrementCountDown, 1.0f, true, 0);
}

void AGM_Arena::DecrementCountDown()
{
	if (iSeconds != 0)
	{
		iSeconds = --iSeconds;

		SendTimerMinutesToGameState();
		SendTimerSecondsToGameState();
	}
	
	if (iMinutes == 0 && iSeconds == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(BeforeMatchTimer);
		MovePlayersToTheirScoutPosition();
		StartScoutPhase();
	}
}

void AGM_Arena::MovePlayersToTheirScoutPosition()
{
	for (int i = 0; i < PlayersInLobby.Num(); ++i)
	{
		if (i == 0) vKillerLocation = FVector(200.0f, 200.0f ,0.f);
		if (i == 1) vKillerLocation = FVector(-200.0f, -200.0f ,0.f);
			
		PlayerToRespawn = PlayersInLobby[i]->GetPawn();
			
		APawn* Pawn = PlayersInLobby[i]->GetPawn();
		Pawn->Reset();
		Pawn->Destroy();
			
		GetDistanceToRespawnBeacons();
		RestartPlayerAtTransform(PlayersInLobby[i], GetSpawnTransform());

		Cast<APlayerBehaviour>(PlayersInLobby[i]->GetPawn())->HideFPModel();
		Cast<APlayerBehaviour>(PlayersInLobby[i]->GetPawn())->SetIsScouting(true);
	}
}

void AGM_Arena::StartScoutPhase()
{
	iMinutes = 0;
	iSeconds = 20;
	
	for (int i = 0; i < PlayersInLobby.Num(); ++i)
	{
		Cast<APlayerBehaviour>(PlayersInLobby[i]->GetPawn())->EnablePlayerInput();
	}
	PlaySound(14);
	GetWorldTimerManager().SetTimer(ScoutPhaseTimer, this, &AGM_Arena::DecrementScoutPhase, 1.0f, true, 0);
}

void AGM_Arena::DecrementScoutPhase()
{
	if (iSeconds != 0)
	{
		iSeconds = --iSeconds;

		SendTimerMinutesToGameState();
		SendTimerSecondsToGameState();
	}
	if (iMinutes == 0 && iSeconds == 7)
		PlaySound(15);

	if (iMinutes == 0 && iSeconds == 5)
		PlaySound(4);
	
	if (iMinutes == 0 && iSeconds == 4)
		PlaySound(3);
	
	if (iMinutes == 0 && iSeconds == 3)
		PlaySound(2);
	
	if (iMinutes == 0 && iSeconds == 2)
		PlaySound(1);
	
	if (iMinutes == 0 && iSeconds == 1)
		PlaySound(0);
	
	if (iMinutes == 0 && iSeconds == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(ScoutPhaseTimer);

		//Set the Spawn the players in oposite sides
		for (int i = PlayersInLobby.Num() - 1; i >= 0; --i)
		{
			if (i == 0) vKillerLocation = FVector(200.0f, 200.0f ,0.f);
			if (i == 1) vKillerLocation = FVector(-200.0f, -200.0f ,0.f);
			
			PlayerToRespawn = PlayersInLobby[i]->GetPawn();
			
			APawn* Pawn = PlayersInLobby[i]->GetPawn();
			if (!Pawn) return;
			
			Pawn->Reset();
			Pawn->Destroy();
			
			GetDistanceToRespawnBeacons();
			RestartPlayerAtTransform(PlayersInLobby[i], GetSpawnTransform());
        	Cast<APlayerBehaviour>(PlayersInLobby[i]->GetPawn())->ShowFPModel();
		  Cast<APlayerBehaviour>(PlayersInLobby[i]->GetPawn())->SetIsScouting(false);
		}

		StartMatch();
	}
}

void AGM_Arena::StartMatch()
{
	iMinutes = iMatchTimeMinutes;
	iSeconds = iMatchTimeSeconds;
	
	
	GetWorldTimerManager().SetTimer(InGameMatchTimer, this, &AGM_Arena::DecrementTime, 1.0f, true, 0);
}

void AGM_Arena::DecrementTime()
{
	if (iSeconds != 0)
	{
		iSeconds = --iSeconds;

		SendTimerSecondsToGameState();
	}
	else
	{
		if (iMinutes == 0)
		{
			//1 minute left
		}
		else
		{
			iMinutes = --iMinutes;
			iSeconds = 59;
			SendTimerMinutesToGameState();
			SendTimerSecondsToGameState();
		}
	}

	if (iMinutes == 4 && iSeconds == 59)
		PlaySound(8);
	
	//1 Minute after the games starts Start Spawning Drops every x Seconds
	if(iMinutes == 4 && iSeconds == 0)
		GetWorld()->GetTimerManager().SetTimer(fTHSpawnDrops, this, &AGM_Arena::SkyDropsStageTwo, 10.0f, true);
	
	if (iMinutes == 3 && iSeconds == 10)
		PlaySound(5);
	
	if ( iMinutes == 3 && iSeconds == 0)
		SkyDropsStageThree();

	if (iMinutes == 2 && iSeconds == 10)
		PlaySound(5);
	
	if ( iMinutes == 2 && iSeconds == 0)
		SkyDropsStageFour();
	
	if ( iMinutes == 1 && iSeconds == 0)
	{
		PlaySound(10);
		ResetPods();
	}

	if ( iMinutes == 0 && iSeconds == 30)
		PlaySound(19);

	if ( iMinutes == 0 && iSeconds == 10)
		PlaySound(18);

	if ( iMinutes == 0 && iSeconds == 5)
		PlaySound(4);

	if ( iMinutes == 0 && iSeconds == 4)
		PlaySound(3);
	
	if ( iMinutes == 0 && iSeconds == 3)
		PlaySound(2);
	
	if ( iMinutes == 0 && iSeconds == 2)
		PlaySound(1);

	if ( iMinutes == 0 && iSeconds == 1)
		PlaySound(0);
	
	if (iMinutes == 0 && iSeconds == 0)
	{
		PlaySound(20);
		
		if (!bIsWaitingToReturnTOMainMenu)
		{
			bIsWaitingToReturnTOMainMenu = true;

			GetWorld()->GetTimerManager().ClearTimer(InGameMatchTimer);

			for (int i = 0; i < PlayersInLobby.Num(); ++i)
			{
				Cast<APlayerBehaviour>(PlayersInLobby[i]->GetPawn())->DisablePlayerInput();
			}
			
			EndMatchTimer();
			ChooseWinner();
		}
	}
}

void AGM_Arena::EndMatchTimer()
{
	iMinutes = 0;
	iSeconds = 30;
	GetWorld()->GetTimerManager().SetTimer(PostMatchTimer, this, &AGM_Arena::DecrementPostMatchTimer, 1.0f, true, 0.0f);
}

void AGM_Arena::DecrementPostMatchTimer()
{
	if (iSeconds != 0)
	{
		iSeconds = --iSeconds;

		SendTimerMinutesToGameState();
		SendTimerSecondsToGameState();
	}
		
	if (iMinutes == 0 && iSeconds == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(PostMatchTimer);
		EndMatch();
	}
}

void AGM_Arena::EndMatch()
{
	//This will trigger the LogOut
	Cast<APlayerBehaviour>(PlayersInLobby[0]->GetPawn())->ReturnToMainMenu();
}

void AGM_Arena::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (Exiting)
	{
		if (Exiting == PlayersInLobby[0])
		{
			//reversed so we start by kicking the clients
			for (int i = PlayersInLobby.Num() - 1; i >= 0; --i)
			{
				UClientDataAndServerSessions* GameInstanceRef = Cast<UClientDataAndServerSessions>(UGameplayStatics::GetGameInstance(GetWorld()));
				if (!GameInstanceRef) return;
				
				//Handle Network errors due to host disconnecting and client losing his connection to him
				GameInstanceRef->HandleNetworkFailure(GetWorld(), GetNetDriver() ,ENetworkFailure::ConnectionLost, "Your connection to the host has been lost.");
			}
		}
	}
}

void AGM_Arena::PlaySound(int SoundToPlayInIndex)
{
	Cast<APlayerBehaviour>(PlayersInLobby[0]->GetPawn())->ChooseAndPlaySound(SoundToPlayInIndex);
	Cast<APlayerBehaviour>(PlayersInLobby[1]->GetPawn())->ChooseAndPlaySound(SoundToPlayInIndex);
}

#pragma endregion MatchTimers
