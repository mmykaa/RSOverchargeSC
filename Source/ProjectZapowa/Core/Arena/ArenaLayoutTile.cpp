// Fill out your copyright notice in the Description page of Project Settings.


#include "ArenaLayoutTile.h"
#include "ProjectZapowa/Core/SkyDrops/DropEscapePod.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectZapowa/Core/SkyDrops/DropAltarPod.h"


// Sets default values
AArenaLayoutTile::AArenaLayoutTile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	iEscapePodType = 0;
	iAltarPodType = 1;

	bPodExists = true;
	bAltarExists = true;

	iEscapePodLocationIndex = 0;
	
	fEscapePodSpawnHeight = 15000.0f;
	
	fLandingInterpSpeedInAir = 1.0f;
	fLandingInterpSpeedOnLanding = 0.6f;
	fCurrentLandingInterpSpeed = fLandingInterpSpeedInAir;
	bIsNearGround = false;
	bIsDropLanded = false;
}

// Called when the game starts or when spawned
void AArenaLayoutTile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AArenaLayoutTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (aDropToTravel != NULL && !bIsDropLanded)
	{
		DropTravelling();
	}
}


bool AArenaLayoutTile::CheckForSlots()
{
	bool bHaveFreeSlots = false;
	
	if (iEscapePodLocationIndex > aEscapePodLandingLocations.Num()-1)
	{
		bHaveFreeSlots = false;
	}

	if (iEscapePodLocationIndex <= aEscapePodLandingLocations.Num()-1)
	{
		bHaveFreeSlots = true;
	}

	return  bHaveFreeSlots;
}

void AArenaLayoutTile::CheckIfDropsExists()
{
	//Check If Drops Exists
	if (aEscapePod != nullptr)
	{
		bPodExists = true;
	}
	else
	{
		bPodExists = false;
		UE_LOG(LogTemp, Warning, TEXT("NO POD ARENA: %s"),* this->GetName());
	}

	if (aAltarPod != nullptr)
	{
		bAltarExists = true;
	}
	else
	{
		bAltarExists = false;
		UE_LOG(LogTemp, Warning, TEXT("NO ALTAR ARENA: %s"),* this->GetName());
	}
}

void AArenaLayoutTile::ChooseDrop(int Type)
{
	//Choose Drop to be spawned
	if (bPodExists)
	{
		if (Type == iEscapePodType)
		{
			if (aEscapePod != nullptr)
			{
				SpawnDrop(aEscapePod, aEscapePodLandingLocations[iEscapePodLocationIndex]);
				++iEscapePodLocationIndex;
			}
		}
	}
	if (bAltarExists)
	{
		if (Type == iAltarPodType)
		{
			if (aAltarPod != nullptr)
			{
				SpawnDrop(aAltarPod, vAltarLandingLocation);
			}
		}
	}
}


void AArenaLayoutTile::SpawnDrop(TSubclassOf<AActor> aDropToSpawn, FVector vLandingLocation)
{

	//Spawn the drop in his landing location plus the desired height
	FRotator rSpawnRotation;
	rSpawnRotation.Yaw = 0.0f;
	rSpawnRotation.Pitch = 0.0f;
	rSpawnRotation.Roll = 0.0f;

	FVector vTileWorldLocation = FVector(this->GetActorLocation().X, this->GetActorLocation().Y, this->GetActorLocation().Z);
	FVector vSpawnOffset = FVector(0.0f, 0.0f, fEscapePodSpawnHeight);
	FVector vFinalSpawnLocation = vLandingLocation + vTileWorldLocation + vSpawnOffset;
	
	
	if (GetWorld() != nullptr)
	{
		AActor* SpawnedDrop = GetWorld()->SpawnActor<AActor>(aDropToSpawn, vFinalSpawnLocation , rSpawnRotation);
		
		aDropToTravel = SpawnedDrop;
		vDropLandingSpot = vFinalSpawnLocation - vSpawnOffset;
		fInitialDistance = FVector::Dist(vFinalSpawnLocation, vDropLandingSpot);
		bIsDropLanded = false;
	}
}

void AArenaLayoutTile::DropTravelling()
{
	//Move the Drop to the desired location

	FVector vCurrentInterpolatedPosition = UKismetMathLibrary::VInterpTo(aDropToTravel->GetActorLocation(), vDropLandingSpot,
															GetWorld()->GetDeltaSeconds(), fCurrentLandingInterpSpeed);
	
	float fCurrentDistanceFromLandingSpot = FVector::Dist(vCurrentInterpolatedPosition, vDropLandingSpot);

	//Mid Air Speed Adjustment
	if(fCurrentDistanceFromLandingSpot <= fInitialDistance/4.0f && !bIsNearGround)
	{
		fCurrentLandingInterpSpeed = fLandingInterpSpeedOnLanding;
	}

	//Near Land Speed Adjustment
	if(fCurrentDistanceFromLandingSpot <= fInitialDistance/10.0f)
	{
		bIsNearGround = true;

		fCurrentLandingInterpSpeed = fLandingInterpSpeedOnLanding * 2;
		
		//Run Opening Animation Here
	}
		
		
	aDropToTravel->SetActorLocation(vCurrentInterpolatedPosition);

	//Check if the landing position nearlu matches the desired one
	if (vCurrentInterpolatedPosition.FVector::Equals(vDropLandingSpot, 20.0f))
	{
		//Stop the Interpolation
		bIsDropLanded = true;
		DropOnLanding();
	}
	

}

void AArenaLayoutTile::DropOnLanding()
{
	//When Landed remove the safe zone
	

	//Run OnLanded Animation Here
	ActivateDrop();
}

void AArenaLayoutTile::ActivateDrop()
{
	
}