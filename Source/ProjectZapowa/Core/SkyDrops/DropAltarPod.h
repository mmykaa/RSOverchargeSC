// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "DropAltarPod.generated.h"


UCLASS()
class PROJECTZAPOWA_API ADropAltarPod : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADropAltarPod();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) USceneComponent* DefaultRoot;
	
	/** Stores the altarPod Mesh */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess = "true"))	USkeletalMeshComponent* SkmAltarMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess = "true"))	USkeletalMeshComponent* SKShardOne;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess = "true"))	USkeletalMeshComponent* SKShardTwo	;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess = "true"))	USkeletalMeshComponent* SKShardThree;

	UPROPERTY(BlueprintReadWrite) bool bIsLanding;
	UFUNCTION() void SetLanding();
	UFUNCTION(BlueprintCallable) bool GetLanding() {return bIsLanding;}
	/** Stores a Cone Mesh that will be used to push the player out of the zone that the drop will land */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)	UStaticMeshComponent* SMSafeZone;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UBoxComponent * Box;

	UPROPERTY() bool bFoundActor;
	UPROPERTY() int iAmountOfShards;
	UFUNCTION() void PlayLandAnimation();
	
	UFUNCTION() void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
	UFUNCTION()	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(Server, Unreliable) void Server_Rep_Landing();


};

