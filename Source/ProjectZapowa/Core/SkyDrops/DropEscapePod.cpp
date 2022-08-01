// Fill out your copyright notice in the Description page of Project Settings.


#include "DropEscapePod.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "ProjectZapowa/Core/Player/PlayerBehaviour.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
ADropEscapePod::ADropEscapePod()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	DefaultRoot->SetMobility(EComponentMobility::Movable);
	SetRootComponent(DefaultRoot);
	
	SKMPodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EscapePodsMesh"));
	SKMPodMesh->SetupAttachment(DefaultRoot);
	
	SKMShard = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMShard"));
    SKMShard->SetupAttachment(DefaultRoot);

	SMSafeZone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SafeZoneMesh"));
	SMSafeZone->SetupAttachment(SKMPodMesh);
	
	Box=CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));

	Box->OnComponentBeginOverlap.AddDynamic(this, &ADropEscapePod::OnBoxBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &ADropEscapePod::OnBoxEndOverlap);
	Box->SetupAttachment(SKMPodMesh);


}

// Called when the game starts or when spawned
void ADropEscapePod::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADropEscapePod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADropEscapePod::ResetPod()
{
	bFoundActor = false;
	SKMShard->SetHiddenInGame(false);
}

void ADropEscapePod::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APlayerBehaviour>(OtherActor) && !bFoundActor)
	{
		/** Check if the player has a shard already */
		if (!Cast<APlayerBehaviour>(OtherActor)->CheckIfIsHoldingShard())
		{
			bFoundActor = true;
			SKMShard->SetHiddenInGame(true);
			Cast<APlayerBehaviour>(OtherActor)->Server_Rep_PlayerPickup(true, OtherActor);
			UE_LOG(LogTemp,Warning, TEXT("ACTOR FOUND HOLDING: %s"), *OtherActor->GetName());
		}
		else
		{
			bFoundActor = false;
		}
	}
}

void ADropEscapePod::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APlayerBehaviour>(OtherActor) && bFoundActor)
	{
		//Nothing i guess
		//bFoundActor = false;
	}
}


