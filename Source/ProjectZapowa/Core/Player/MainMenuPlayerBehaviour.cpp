// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectZapowa/Core/Player/MainMenuPlayerBehaviour.h"

#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Kismet/GameplayStatics.h"
#include "ProjectZapowa/Core/TargetDummy.h"
#include "ProjectZapowa/Core/Networking/ClientDataAndServerSessions.h"


// Sets default values
AMainMenuPlayerBehaviour::AMainMenuPlayerBehaviour()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
	//Player Collisions
	fCapsuleSizeX = 42.0f;
	fCapsuleSizeY = 96.0f;


		//UI & Settings
	fCameraSensitivity = 0.5f;
	bIsCameraYInverted = false;
	
	//Player Movement Base
	fAcceleration = 4096.0;
	fBreakingFactor = 0.8f;
	fJumpForce = 650.0f;
	fAirMovementControl = 0.8f;
	fPlayerGravity = 2.0f;
	bCanPlayerMove = false;
	
	//Player Overcharge
	iOverchargeStage = 0;
	
	//First Person Camera
	fPlayerEyeHeight = 80.0f;
	vFirstPersonCameraPosition = FVector(10.0f, 0, fPlayerEyeHeight);
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	PlayerCamera->SetRelativeLocation(vFirstPersonCameraPosition);
	PlayerCamera->bUsePawnControlRotation = true;
	fMaxCameraStrafeTilt = 0.7f;
	fStrafeTiltInterpolationSpeed = 4.0f;
	
	//Main Menu Cameras
	MMCameraZero = CreateDefaultSubobject<USceneComponent>(TEXT("CameraZeroSpot"));
	MMCameraZero->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	MMCameraOne = CreateDefaultSubobject<USceneComponent>(TEXT("CameraOneSpot"));
	MMCameraOne->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	MMCameraTwo = CreateDefaultSubobject<USceneComponent>(TEXT("CameraTwoSpot"));
	MMCameraTwo->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	MMCameraThree = CreateDefaultSubobject<USceneComponent>(TEXT("CameraThreeSpot"));
	MMCameraThree->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	MMCameraFour = CreateDefaultSubobject<USceneComponent>(TEXT("CameraFourSpot"));
	MMCameraFour->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	MMCameraFive = CreateDefaultSubobject<USceneComponent>(TEXT("CameraFiveSpot"));
	MMCameraFive->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	MMCameraSix = CreateDefaultSubobject<USceneComponent>(TEXT("CameraSixSpot"));
	MMCameraSix->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	
	//Player Weapon
	ShootingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ShootPoint"));
	ShootingPoint->SetupAttachment(PlayerCamera);
	iMaxAmmo = 2;
	iCurrentAmmo = iMaxAmmo;
	fShotgunFireRate = 0.25f;
	fDefaultShotgunReloadTime = 2.0f;
	fCurrentShotgunReloadTime = fDefaultShotgunReloadTime;
	bCanFireShotgun = true;
	iCenterDamage = 50;
	iSpreadDamage = 10;
	iMaxSlugOpening = 1500;
	iMinSlugOpening = 500;

	//Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(fCapsuleSizeX, fCapsuleSizeY);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 360.0f);
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxAcceleration = fAcceleration;
	GetCharacterMovement()->JumpZVelocity = fJumpForce;
	GetCharacterMovement()->AirControl = fAirMovementControl;
	GetCharacterMovement()->GravityScale = fPlayerGravity;
	GetCharacterMovement()->BrakingFrictionFactor = fBreakingFactor;


	//Mesh
	SKMTPCharacter = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMTPCharacter"));
	SKMTPCharacter->SetupAttachment(RootComponent);
	SKMTPCharacter->SetOwnerNoSee(true);
	SKMTPCharacter->SetOnlyOwnerSee(false);
	
	SKMTPWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SKMTPWeapon"));
	SKMTPWeapon->SetOwnerNoSee(true);
	SKMTPWeapon->SetOnlyOwnerSee(false);
		
	SKViewModel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViewModel"));
	SKViewModel->SetupAttachment(PlayerCamera);
	SKViewModel->SetOnlyOwnerSee(true);
	SKViewModel->SetOwnerNoSee(false);
	
	//Sound
	ACReload = CreateDefaultSubobject<UAudioComponent>(TEXT("ReloadSounds"));
	ACReload->SetupAttachment(SKViewModel);
}

void AMainMenuPlayerBehaviour::ParticleAndSound(UNiagaraSystem* ParticleToReplicate, USoundBase* SoundToReplicate,
	FVector InLocation, FRotator InRotation)
{
	if (SoundToReplicate)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToReplicate, InLocation, InRotation,1.0f);
	}

	if (ParticleToReplicate)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ParticleToReplicate ,InLocation,InRotation);
	}
}

// Called when the game starts or when spawned
void AMainMenuPlayerBehaviour::BeginPlay()
{
	Super::BeginPlay();
	
	iMaxOvercharges = fMovementSpeedOnStage.Num() - 1;
	ApplyOvercharge();
	SKMTPWeapon->AttachToComponent(SKMTPCharacter, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName(TEXT("TPWeaponSocket")));

	DynamicMat1=UMaterialInstanceDynamic::Create(SKMTPCharacter->GetMaterial(1), this);
	SKMTPCharacter->SetMaterial(1, DynamicMat1);
	DynamicMat5=UMaterialInstanceDynamic::Create(SKMTPCharacter->GetMaterial(5), this);
	SKMTPCharacter->SetMaterial(5, DynamicMat5);
	DynamicMat10=UMaterialInstanceDynamic::Create(SKMTPCharacter->GetMaterial(10), this);
	SKMTPCharacter->SetMaterial(10, DynamicMat10);
	
	
	DynamicMatView0=UMaterialInstanceDynamic::Create(SKViewModel->GetMaterial(0), this);
	SKViewModel->SetMaterial(0, DynamicMatView0);
	DynamicMatView5=UMaterialInstanceDynamic::Create(SKViewModel->GetMaterial(5), this);
	SKViewModel->SetMaterial(5, DynamicMatView5);
	DynamicMatView9=UMaterialInstanceDynamic::Create(SKViewModel->GetMaterial(9), this);
	SKViewModel->SetMaterial(9, DynamicMatView9);

	SetOvercharge(0);

	//Destroying the session if available, so the player can create another
	
	UClientDataAndServerSessions* GameInstanceRef = Cast<UClientDataAndServerSessions>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!GameInstanceRef) return;
	GameInstanceRef->DestroySession();
}

// Called every frame
void AMainMenuPlayerBehaviour::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMainMenuPlayerBehaviour::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainMenuPlayerBehaviour::MoveForward);
	PlayerInputComponent->BindAxis("MoveSideways", this, &AMainMenuPlayerBehaviour::MoveSideways);
	PlayerInputComponent->BindAction("Overcharge", IE_Pressed, this, &AMainMenuPlayerBehaviour::Overcharge);
	//PlayerInputComponent->BindAction("Discharge", IE_Pressed, this, &AMainMenuPlayerBehaviour::Discharge);
	PlayerInputComponent->BindAxis("Turn", this, &AMainMenuPlayerBehaviour::HorizontalMouseCameraMovement);
	PlayerInputComponent->BindAxis("LookUp", this, &AMainMenuPlayerBehaviour::VerticalMouseCameraMovement);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainMenuPlayerBehaviour::HorizontalJoystickCameraMovement);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainMenuPlayerBehaviour::VerticalJoystickCameraMovement);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainMenuPlayerBehaviour::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMainMenuPlayerBehaviour::EndJump);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMainMenuPlayerBehaviour::StartAttack);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMainMenuPlayerBehaviour::ReloadShotgun);

	PlayerInputComponent->BindAction("UsingController", IE_Pressed, this, &AMainMenuPlayerBehaviour::UpdateInputToController);
	PlayerInputComponent->BindAction("UsingKeyboard", IE_Pressed, this, &AMainMenuPlayerBehaviour::UpdateInputToKeyboard);
}


#pragma region UIAndWidgets

void AMainMenuPlayerBehaviour::SetPlayerSettings(float Sensitivity, bool CameraYInverted)
{
	// 	fMouseSensitivity = MouseSensitivity;
	// fJoystickSensitivity = JoystickSensitivity;
	// 	bIsMouseInverted = MouseInverted;
	// 	bUseAutoReload = AutoReloadWeapon;
}

void AMainMenuPlayerBehaviour::UpdateInputToController()
{
	bIsUsingController = true;
}

void AMainMenuPlayerBehaviour::UpdateInputToKeyboard()
{
	bIsUsingController = false;
}

bool AMainMenuPlayerBehaviour::IsUsingController()
{
	return bIsUsingController;
}


void AMainMenuPlayerBehaviour::PlayReloadingWidgetToViewport()
{
	if (wReloadingClass == nullptr)
	{
		return;
	}

	wReloadingWidget = CreateWidget<UUserWidget>(GetWorld(), wReloadingClass);

	if (wReloadingWidget == nullptr)
	{
		return;
	}

	wReloadingWidget->AddToViewport();
}

void AMainMenuPlayerBehaviour::RemoveReloadingWidgetFromViewport() const
{
	wReloadingWidget->RemoveFromViewport();
}

#pragma endregion UIAndWidgets

#pragma region PlayerCameraMovement

void AMainMenuPlayerBehaviour::HorizontalMouseCameraMovement(float Value)
{
	if (bIsInMainMenu)
	{
		return;
	}

	if (Value != 0)
	{
		bIsUsingController = false;

		AddControllerYawInput(Value * fCameraSensitivity);
	}
}

void AMainMenuPlayerBehaviour::VerticalMouseCameraMovement(float Value)
{
	if (bIsInMainMenu)
	{
		return;
	}

	if (Value != 0)
	{
		bIsUsingController = false;
		
		if (!bIsCameraYInverted)
		{
			AddControllerPitchInput(Value * fCameraSensitivity);
		}
		else
		{
			AddControllerPitchInput(-Value * fCameraSensitivity);
		}
	}

}

void AMainMenuPlayerBehaviour::HorizontalJoystickCameraMovement(float Value)
{
	if (bIsInMainMenu)
	{
		return;
	}

	if (Value != 0)
	{
		bIsUsingController = true;

		AddControllerYawInput(Value * fCameraSensitivity);
	}
}

void AMainMenuPlayerBehaviour::VerticalJoystickCameraMovement(float Value)
{
	if (bIsInMainMenu)
	{
		return;
	}

	if (Value != 0)
	{
		bIsUsingController = true;

		if (!bIsCameraYInverted)
		{
			AddControllerPitchInput(Value * fCameraSensitivity);
		}
		else
		{
			AddControllerPitchInput(-Value * fCameraSensitivity);
		}
	}
}

void AMainMenuPlayerBehaviour::StrafeCameraTilt(float Value) const
{
	const FRotator rCurrentRotation = Controller->GetControlRotation();
	const FRotator rTarget = FRotator(Controller->GetControlRotation().Pitch, Controller->GetControlRotation().Yaw,
	                               fMaxCameraStrafeTilt * Value);
	const FRotator rStrafeTilt = FMath::RInterpTo(rCurrentRotation, rTarget, GetWorld()->GetDeltaSeconds(),
	                                           fStrafeTiltInterpolationSpeed);

	Controller->SetControlRotation(rStrafeTilt);
}

void AMainMenuPlayerBehaviour::PlayerIsUsingUI()
{
	if (bCanUseMainMenu)
	{
		bIsInMainMenu = true;
		bCanUseMainMenu = false;
	}
}

void AMainMenuPlayerBehaviour::PlayerIsNotUsingUI()
{
	if (!bCanUseMainMenu)
	{
		bIsInMainMenu = false;
		bCanUseMainMenu = true;
	}
}

void AMainMenuPlayerBehaviour::SwitchToFirstPersonCamera()
{
	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(this, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show first person mesh

	//hide third person mesh	
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraZero()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraZero->GetComponentLocation(),
	                                                        MMCameraZero->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraOne()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraOne->GetComponentLocation(),
	                                                        MMCameraOne->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraTwo()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraTwo->GetComponentLocation(),
	                                                        MMCameraTwo->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraThree()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraThree->GetComponentLocation(),
	                                                        MMCameraThree->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraFour()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraFour->GetComponentLocation(),
	                                                        MMCameraFour->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraFive()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraFive->GetComponentLocation(),
	                                                        MMCameraFive->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

void AMainMenuPlayerBehaviour::SwitchToMainMenuCameraSix()
{
	if (MainMenuCamera == nullptr)
	{
		return;
	}

	MainMenuCameraToTravel = GetWorld()->SpawnActor<AActor>(MainMenuCamera, MMCameraSix->GetComponentLocation(),
	                                                        MMCameraSix->GetComponentRotation());

	APlayerController* PController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	PController->SetViewTargetWithBlend(MainMenuCameraToTravel, 0.4f, VTBlend_Cubic, 1.0f, false);

	//show third person mesh

	//hide first person mesh
}

#pragma endregion PlayerCameraMovement

#pragma region PlayerBaseMovement

void AMainMenuPlayerBehaviour::MoveForward(float Value)
{
	if (bIsInMainMenu)
	{
		return;
	}

	//Find out which way is forward and record that the player wants to move that way
	const FVector Direction = GetActorForwardVector();

	if (Value != 0)
	{
		AddMovementInput(Direction, Value);
	}
}

void AMainMenuPlayerBehaviour::MoveSideways(float Value)
{
	if (bIsInMainMenu)
	{
		return;
	}

	//Find out which way is Sideways, and record that the player wants to move that way
	const FVector Direction = GetActorRightVector();

	if (Value != 0)
	{
		AddMovementInput(Direction, Value);
		StrafeCameraTilt(Value);
	}
}

void AMainMenuPlayerBehaviour::StartJump()
{
	if (bIsInMainMenu)
	{
		return;
	}
	if (bPressedJump) return;
	
	CheckForOverchargeStage();

	bPressedJump = true;
	JumpingFeedback();

	CheckIfPlayerIsLanded();
}

void AMainMenuPlayerBehaviour::EndJump()
{
	bPressedJump = false;
}

void AMainMenuPlayerBehaviour::CheckIfPlayerIsLanded()
{
	GetWorld()->GetTimerManager().SetTimer(fthCheckForGround, this, &AMainMenuPlayerBehaviour::CheckIfPlayerIsLanded, 0.01f,
	                                       false);

	if (GetCharacterMovement()->IsFalling())
	{
		bCanUseMainMenu = false;
	}

	if (!GetCharacterMovement()->IsFalling())
	{
		GetWorld()->GetTimerManager().ClearTimer(fthCheckForGround);
		ResetOverchargeValues();
		bCanUseMainMenu = true;
	}
}

#pragma endregion PlayerBaseMovement

#pragma region Feedback

void AMainMenuPlayerBehaviour::ShootingFeedback()
{
	/** Play Animation */
	if (FireAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(FireAnimation, 1.5f);
	}
	
	ParticleAndSound(nullptr, ShootingSounds[iOverchargeStage], this->GetActorLocation(), this->GetControlRotation());

	//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),NSMuzzleFlash,ShootingPoint->GetComponentLocation(), this->GetControlRotation() + FRotator(90.0f, 00.0f, 90.0f));
    SpawnMuzzleFlash();
	/** Play Cammera Shake */
	UGameplayStatics::PlayWorldCameraShake(GetWorld(),CSShooting,this->GetActorLocation(),0.0f,200.0f, 1.0f,false);
	
}

void AMainMenuPlayerBehaviour::JumpingFeedback()
{
	/** Play Animation */
	if (JumpAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(JumpAnimation, 1.0f);
	}

	/** Play Sound */
	//UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShootingSounds[iOverchargeStage],this->GetActorLocation(), this->GetControlRotation(),1.0f);
}

void AMainMenuPlayerBehaviour::ReloadFeedback()
{
	if (ReloadAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(ReloadAnimation, 1.0f);
	}
	
	ACReload->SetIntParameter("WeaponReloadNoise", iOverchargeStage);
	ACReload->Play();

	
}

#pragma endregion Feedback

#pragma region Overcharge

void AMainMenuPlayerBehaviour::Overcharge()
{
	if (iOverchargeStage >= iMaxOvercharges)
		return;

	
	++iOverchargeStage;
	SetOvercharge(iOverchargeStage);
	ApplyOvercharge();
}

int AMainMenuPlayerBehaviour::AmmoToUI()
{
	return iCurrentAmmo;
}

void AMainMenuPlayerBehaviour::SetOvercharge(int iOverchargeValue)
{
	iOverchargeStage = iOverchargeValue;
		
		if (iOverchargeStage == 0)
    	{
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.0f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.1875f, 0.1875f, 0.1875f));
    		
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.0f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.1875f, 0.1875f, 0.1875f));
    			
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.0f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.1875f, 0.1875f, 0.1875f));
    
    		DynamicMatView5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.0f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.1875f, 0.1875f, 0.1875f));
    		
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.0f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.1875f, 0.1875f, 0.1875f));
    
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.0f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.1875f, 0.1875f, 0.1875f));
    	}
    
    	if (iOverchargeStage == 1)
    	{
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.3f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"), FVector( 0.5f, 0.47f, 0.32f));
    		
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.3f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"), FVector( 0.5f, 0.47f, 0.32f));
    		
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.3f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector( 0.5f, 0.47f, 0.32f));
    		
    		DynamicMatView5->SetScalarParameterValue(TEXT("BaseEmissiveOutputIntensity"), 0.3f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector( 0.5f, 0.47f, 0.32f));
    		
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.3f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector( 0.5f, 0.47f, 0.32f));
    		
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 0.3f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector( 0.5f, 0.47f, 0.32f));
    	}
    	
    	if (iOverchargeStage == 2)
    	{
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 5.0f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.93f, 0.83f, 0.37f));
    
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 5.0f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.93f, 0.83f, 0.37f));
    	
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 5.0f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.93f, 0.83f, 0.37f));
    		
    		DynamicMatView5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 5.0f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.93f, 0.83f, 0.37f));
    		
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 5.0f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.93f, 0.83f, 0.37f));
    
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 5.0f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector(0.93f, 0.83f, 0.37f));
    	}	
    
    	if (iOverchargeStage == 3)
    	{
    		
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 20.0f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.85f, 0.14f));
    		
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 20.0f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"),FVector( 1.0f, 0.85f, 0.14f));
    	
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 20.0f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.85f, 0.14f));
    
    		DynamicMatView5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 20.0f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.85f, 0.14f));
    
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 20.0f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.85f, 0.14f));
    
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 20.0f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.85f, 0.14f));
    
    	}
    
    	if (iOverchargeStage == 4)
    	{
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 40.0f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.7f, 0.0f));
    
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 40.0f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.7f, 0.0f));
    	
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 40.0f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.7f, 0.0f));
    
    		DynamicMatView5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 40.0f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.7f, 0.0f));
    
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 40.0f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.7f, 0.0f));
    
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 40.0f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.7f, 0.0f));
    
    	}
    
    	if (iOverchargeStage == 5)
    	{
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 70.0f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.5f, 0.0f));
    
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 70.0f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.5f, 0.0f));
    	
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 70.0f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.5f, 0.0f));
    		
    		DynamicMatView5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 70.0f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.5f, 0.0f));
    
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 70.0f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.5f, 0.0f));
    		
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 70.0f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.5f, 0.0f));
    	}
    
    	if (iOverchargeStage == 6)
    	{
    		DynamicMat1->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 100.0f);
    		DynamicMat1->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.4f, 0.0f));
    
    		DynamicMat5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 100.0f);
    		DynamicMat5->SetVectorParameterValue(TEXT("BaseColor"),  FVector( 1.0f, 0.4f, 0.0f));
    	
    		DynamicMat10->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 100.0f);
    		DynamicMat10->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.4f, 0.0f));
    
    		DynamicMatView5->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"),	100.0f);
    		DynamicMatView5->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.4f, 0.0f));
    
    		DynamicMatView0->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 100.0f);
    		DynamicMatView0->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.4f, 0.0f));
    		
    		DynamicMatView9->SetScalarParameterValue(TEXT("Base Emissive Output Intensity"), 100.0f);
    		DynamicMatView9->SetVectorParameterValue(TEXT("BaseColor"), FVector( 1.0f, 0.4f, 0.0f));
    	}	
}

void AMainMenuPlayerBehaviour::Discharge()
{
	if (iOverchargeStage == 0)
	{
		return;
	}
	
	--iOverchargeStage;
	SetOvercharge(iOverchargeStage);
	ApplyOvercharge();
}

void AMainMenuPlayerBehaviour::ResetOverchargeValues()
{
	ApplyOvercharge();
}

void AMainMenuPlayerBehaviour::ApplyOvercharge()
{
	GetCharacterMovement()->MaxWalkSpeed = fMovementSpeedOnStage[iOverchargeStage];
}

void AMainMenuPlayerBehaviour::CheckForOverchargeStage()
{
	GetCharacterMovement()->MaxWalkSpeed = fJumpDistanceOnStage[iOverchargeStage];
}

#pragma endregion Overcharge

#pragma region PlayerShooting

void AMainMenuPlayerBehaviour::StartAttack()
{
	if (bIsInMainMenu)
	{
		return;
	}

	if (iCurrentAmmo == 0 && !bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("AUTO RELOAD"));

		bCanFireShotgun = false;

		ReloadShotgun();
	}

	if (iCurrentAmmo > 0 && !bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("CAN SHOOT"));

		UseShotgun();
	}
}

void AMainMenuPlayerBehaviour::UseShotgun()
{
	if (bCanFireShotgun && !bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("SHOOTING"));

		bCanFireShotgun = false;
		//Play Firing Animation
		--iCurrentAmmo;
		ShootingFeedback();
		//Update Ammo Amount in UI
		FireShotgun();
		
		if (iCurrentAmmo == 0)
		    ReloadShotgun();
	}
	else if (!bCanFireShotgun)
	{
		UE_LOG(LogTemp, Warning, TEXT("On Cooldown"));
	}
}

void AMainMenuPlayerBehaviour::FireShotgun() //SHOULD BE Called by a notify in the shooting animation
{
	FVector vForwardVector = PlayerCamera->GetForwardVector();

	FVector vStart = ShootingPoint->GetComponentLocation();

	FVector vEnd = (vStart + (vForwardVector * 20000.0f));


	FHitResult OutHitMain;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	//DrawDebugLine(GetWorld(), vStart, vEnd, FColor::Black, true);

	GetWorld()->LineTraceSingleByChannel(OutHitMain, vStart, vEnd, ECC_Visibility, CollisionParams);

	if (Cast<ATargetDummy>(OutHitMain.GetActor()))
	{
		Cast<ATargetDummy>(OutHitMain.GetActor())->SitDown();
	}
	else if (OutHitMain.GetActor())
	{
		ParticleAndSound(NSHitSpark, nullptr, OutHitMain.Location, GetControlRotation() * -1);
	}

	if (!OutHitMain.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Missed"));
	}
	
	

	if (bUseAutoReloadByOvercharge && iCurrentAmmo == 0)
	{
		ReloadShotgun();
	}

	GetWorld()->GetTimerManager().SetTimer(fthShotgunTimerHandle, this, &AMainMenuPlayerBehaviour::ResetShotgunFireRate, fShotgunFireRate, false);
}


void AMainMenuPlayerBehaviour::ReloadShotgun()
{
	if (iCurrentAmmo == iMaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("CANT RELOAD"));

		return;
	}

	if (iCurrentAmmo < iMaxAmmo && !bIsReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("RELOADING"));

		bIsReloading = true;

		//0 -> 100% / 1 -> 80% / 2 -> 60% / 3 -> 40% / 4 -> 20% / 5 -> 0% / 6 -> 0%
		fCurrentShotgunReloadTime = (fDefaultShotgunReloadTime - (iOverchargeStage * (0.20f * fDefaultShotgunReloadTime)));

		if (fCurrentShotgunReloadTime <= 0)
		{
			fCurrentShotgunReloadTime = 0.35f;
		}
		ReloadFeedback();

		GetWorld()->GetTimerManager().SetTimer(fthShotgunReloadTimerHandle, this, &AMainMenuPlayerBehaviour::FinishedReload, fCurrentShotgunReloadTime, false);

		PlayReloadingWidgetToViewport();
	}
}

void AMainMenuPlayerBehaviour::FinishedReload()
{
	//GetWorld()->GetTimerManager().ClearTimer(fthShotgunReloadTimerHandle);

	if (iCurrentAmmo < iMaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("FINISHED RELOADING"));

		RemoveReloadingWidgetFromViewport();

		iCurrentAmmo = iMaxAmmo;

		bIsReloading = false;
		bCanFireShotgun = true;
	}
}

void AMainMenuPlayerBehaviour::ResetShotgunFireRate()
{
	bCanFireShotgun = true;
	GetWorld()->GetTimerManager().ClearTimer(fthShotgunTimerHandle);
}

#pragma endregion PlayerShoo