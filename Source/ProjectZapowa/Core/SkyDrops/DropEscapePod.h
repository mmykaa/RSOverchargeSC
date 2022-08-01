// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "DropEscapePod.generated.h"


UCLASS()
class PROJECTZAPOWA_API ADropEscapePod : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADropEscapePod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(BlueprintReadWrite, EditAnywhere) USceneComponent* DefaultRoot;
	
	/** Stores the EscapePod Mesh */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess = "true"))	USkeletalMeshComponent* SKMPodMesh;
	
		UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess = "true"))	USkeletalMeshComponent* SKMShard;


	/** Stores a Cone Mesh that will be used to push the player out of the zone that the drop will land */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	UStaticMeshComponent* SMSafeZone;

	/** A Box Trigger */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UBoxComponent * Box;

	/** Used to lock the Pod */
	UPROPERTY() bool bFoundActor;

	/** Called by the Gamemode at 1 minute, used to reset the pod */
	UFUNCTION() void ResetPod();

	UFUNCTION() void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	
	
};
