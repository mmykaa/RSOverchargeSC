// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZapowa/Core/TargetDummy.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
ATargetDummy::ATargetDummy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	fDownSize = 0.05f;
	fDefaultSize = 1.0f;
	
	fSizeToInterpTo = 0;
}

// Called when the game starts or when spawned
void ATargetDummy::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATargetDummy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (fSizeToInterpTo == fDefaultSize && !UKismetMathLibrary::NearlyEqual_FloatFloat( this->GetActorScale3D().Z, fSizeToInterpTo, 0.01))
	{
		GetUp();
	}
}

void ATargetDummy::SitDown()
{
	SetActorScale3D(FVector(1.0f, 1.0f, fDownSize));
	
	FTimerHandle ResetTimer;
	GetWorldTimerManager().SetTimer(ResetTimer, this, &ATargetDummy::GetUp, 3.0f, true, 0);	
}

void ATargetDummy::GetUp()
{
	fSizeToInterpTo = fDefaultSize;
	float myZScale = UKismetMathLibrary::FInterpTo(GetActorScale3D().Z, fSizeToInterpTo, GetWorld()->DeltaTimeSeconds, 0.3f);
	SetActorScale3D(FVector(1.0f, 1.0f, myZScale));
}

