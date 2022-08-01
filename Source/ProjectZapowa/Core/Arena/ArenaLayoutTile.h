// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArenaLayoutTile.generated.h"

UCLASS()
class PROJECTZAPOWA_API AArenaLayoutTile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AArenaLayoutTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#pragma region DropSpawning

	/** Stores the EscapePod Actor */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	TSubclassOf<AActor> aEscapePod;

	/** Stores the EscapePod Possible Landing Locations  */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	TArray<FVector> aEscapePodLandingLocations;

	/** Sets the current spawning location index */
	UPROPERTY()	int iEscapePodLocationIndex;

	/** Height that the Escape Pod will Spawn */
	UPROPERTY()	float fEscapePodSpawnHeight;

	/** Stores the Altar Actor */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	TSubclassOf<AActor> aAltarPod;

	/** Stores the Altar Landing Location */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	FVector vAltarLandingLocation;

	/** Used to Check if this Tile can Spawn EscapePods */
	UPROPERTY()	bool bPodExists;

	/** Used to Check if this Tile can Spawn Altars */
	UPROPERTY()	bool bAltarExists;

	/** Stores the type of the Drop to spawn, if 0 is an EscapePod  */
	UPROPERTY()	int iEscapePodType;

	/** Stores the type of the Drop to spawn, if 1 is an Altar  */
	UPROPERTY()	int iAltarPodType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FString sTileName;

	UFUNCTION() FString GetTileName() {return sTileName;}
	AActor* aDropToTravel;
	FVector vDropLandingSpot;
	bool bIsDropLanded;

	float fCurrentLandingInterpSpeed;
	float fLandingInterpSpeedInAir;
	float fLandingInterpSpeedOnLanding;
	bool bIsNearGround;
	float fInitialDistance;

	

	
	//////////////////////////////////////////////////////////////////////////

	/** Checks if the Tile have any available slots for drop spawning */
	UFUNCTION()	bool CheckForSlots();
	
	/** Checks if the Tile have Drops To Spawn */
	UFUNCTION()	void CheckIfDropsExists();

	/** Chooses in the Drop To Spawn */
	UFUNCTION()	void ChooseDrop(int Type);

	/** Spawn the drop */
	UFUNCTION()	void SpawnDrop(TSubclassOf<AActor> aDropToSpawn, FVector vLandingLocation);

	
	/** Initiate Drop Travelling the drop */
	UFUNCTION()	void DropTravelling();
	
	/** Start On-Landing procedures */
	UFUNCTION()	void DropOnLanding();
	
	/** Activate the Landed Drop */
	UFUNCTION()	void ActivateDrop();


#pragma endregion DropSpawning
	
};


