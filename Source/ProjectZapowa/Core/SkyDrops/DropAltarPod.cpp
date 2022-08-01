// Fill out your copyright notice in the Description page of Project Settings.


#include "DropAltarPod.h"
#include "Components/BoxComponent.h"
#include "ProjectZapowa/Core/Player/PlayerBehaviour.h"

// Sets default values
ADropAltarPod::ADropAltarPod()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	DefaultRoot->SetMobility(EComponentMobility::Movable);
	
	SetRootComponent(DefaultRoot);
	
	SkmAltarMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AltarMesh"));
	SkmAltarMesh->SetupAttachment(DefaultRoot);

	SKShardOne = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKShardOne"));
	SKShardOne->SetupAttachment(DefaultRoot);

	SKShardTwo = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKShardTwo"));
	SKShardTwo->SetupAttachment(DefaultRoot);
	
	SKShardThree = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKShardThree"));
	SKShardThree->SetupAttachment(DefaultRoot);
	

	SMSafeZone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafeZoneMesh"));
	SMSafeZone->SetupAttachment(SkmAltarMesh);

	Box=CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));

	Box->OnComponentBeginOverlap.AddDynamic(this, &ADropAltarPod::OnBoxBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &ADropAltarPod::OnBoxEndOverlap);
	Box->SetupAttachment(SkmAltarMesh);
}

// Called when the game starts or when spawned
void ADropAltarPod::BeginPlay()
{
	Super::BeginPlay();

	SKShardOne->SetHiddenInGame(true);
	SKShardTwo->SetHiddenInGame(true);
	SKShardThree->SetHiddenInGame(true);
}

// Called every frame
void ADropAltarPod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADropAltarPod::SetLanding()
{
	bIsLanding = true;
}

void ADropAltarPod::PlayLandAnimation()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AUTH"));
		Server_Rep_Landing();	
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO AUTH"));

		Server_Rep_Landing();

	}
}

void ADropAltarPod::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APlayerBehaviour>(OtherActor) && !bFoundActor && HasAuthority())
	{
		if (Cast<APlayerBehaviour>(OtherActor)->CheckIfIsHoldingShard())
		{
			++iAmountOfShards;
			
			if (iAmountOfShards == 1)
				SKShardOne->SetHiddenInGame(false);
			
			if (iAmountOfShards == 2)
				SKShardTwo->SetHiddenInGame(false);

			if (iAmountOfShards > 3)
			{
				iAmountOfShards = 0;
				SKShardThree->SetHiddenInGame(false);
			}
		}
		bFoundActor = true;
		Cast<APlayerBehaviour>(OtherActor)->Client_Rep_PlayerNearAltar(true, OtherActor);
	}
}

void ADropAltarPod::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APlayerBehaviour>(OtherActor) && bFoundActor && HasAuthority())
	{
		bFoundActor = false;
		Cast<APlayerBehaviour>(OtherActor)->Client_Rep_PlayerNearAltar(false, OtherActor);
		if (iAmountOfShards > 3)
		{
			SKShardOne->SetHiddenInGame(true);
			SKShardTwo->SetHiddenInGame(true);
			SKShardThree->SetHiddenInGame(true);
		}
	}
}

void ADropAltarPod::Server_Rep_Landing_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("LANDING %d"), bIsLanding);
	bIsLanding = true;
}




