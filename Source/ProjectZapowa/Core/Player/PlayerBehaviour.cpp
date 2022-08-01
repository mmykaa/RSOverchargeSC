// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBehaviour.h"

#include "DrawDebugHelpers.h"
#include "NiagaraComponentPool.h"
#include "ProjectZapowa/Core/Networking/ClientDataAndServerSessions.h"
#include "ProjectZapowa/Core/UGameModes/GM_Arena.h"

#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Blueprint/UserWidget.h"

#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectZapowa/Core/UGameStates/PS_PlayerStatistics.h"


// Sets default values
APlayerBehaviour::APlayerBehaviour()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Player Collisions
	fCapsuleSizeX = 42.0f;
	fCapsuleSizeY = 96.0f;

	bIsNotMoving = false;
	
	//UI & Settings
	fCameraSensitivity = 0.5f;
	bIsCameraYInverted = false;
	bIsShowingTimer = false;
	fEnemyKillerWidgetDisplayTime = 1.5f;

	
	//Player Movement Base
	fAcceleration = 4096.0;
	fBreakingFactor = 0.8f;
	fJumpForce = 650.0f;
	fAirMovementControl = 0.8f;
	fPlayerGravity = 2.0f;
	bCanPlayerMove = false;
	bCanJump = true;
	
	//Player Overcharge
	iOverchargeStage = 0;
	
	//First Person Camera
	fPlayerEyeHeight = 80.0f;
	vFirstPersonCameraPosition = FVector(10.0f, 0, fPlayerEyeHeight);
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(CastChecked<USceneComponent, UCapsuleComponent>(GetCapsuleComponent()));
	PlayerCamera->SetRelativeLocation(vFirstPersonCameraPosition);
	PlayerCamera->bUsePawnControlRotation = true;
	fMaxCameraStrafeTilt = 1.0f;
	fStrafeTiltInterpolationSpeed = 4.0f;
	fDefaultPlayerFOV = 90.0f;
	
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
	iSpreadDamage = 9;
	iMaxSlugOpening = 1500;
	iMinSlugOpening = 500;
	
	//Player Health
	iMaxHealth = 100;
	iCurrentHealth = iMaxHealth;
	bIsPlayerDead = false;
	bIsSpawnKillProtected = false;
	fSpawnKillProtectionTime = 5.0f;
	
	//Pickups
	iShardsDelivered = 0;
	iMaxShardsDelivered = 3;
	iOverchargeConsumables = 5;
	iMaxOverchargeConsumables = 20;
	
	//Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	bIsOnGround = true;

	GetCapsuleComponent()->InitCapsuleSize(fCapsuleSizeX, fCapsuleSizeY);

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 360.0f);
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxAcceleration = fAcceleration;
	GetCharacterMovement()->JumpZVelocity = fJumpForce;
	GetCharacterMovement()->AirControl = fAirMovementControl;
	GetCharacterMovement()->GravityScale = fPlayerGravity;
	GetCharacterMovement()->BrakingFrictionFactor = fBreakingFactor;

	SKMTPCharacter = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SKMTPCharacter"));
	SKMTPCharacter->SetupAttachment(RootComponent);
	SKMTPCharacter->SetOwnerNoSee(true);
	SKMTPCharacter->SetOnlyOwnerSee(false);
	
	SKMTPWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SKMTPWeapon"));
	SKMTPWeapon->SetupAttachment(RootComponent);
	SKMTPWeapon->SetOwnerNoSee(true);
	SKMTPWeapon->SetOnlyOwnerSee(false);
		
	SKViewModel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ViewModel"));
	SKViewModel->SetupAttachment(PlayerCamera);
	SKViewModel->SetOnlyOwnerSee(true);
	SKViewModel->SetOwnerNoSee(false);
	
	//Sound
	ACReload = CreateDefaultSubobject<UAudioComponent>(TEXT("ReloadSounds"));
	ACReload->SetupAttachment(SKViewModel);

	ACShooting = CreateDefaultSubobject<UAudioComponent>(TEXT("ACShooting"));
	ACShooting->SetupAttachment(SKViewModel);

	ACBase = CreateDefaultSubobject<UAudioComponent>(TEXT("ACBase"));
	ACBase->SetupAttachment(SKViewModel);
	
	ACBeat = CreateDefaultSubobject<UAudioComponent>(TEXT("ACBeat"));
	ACBeat->SetupAttachment(SKViewModel);
	
	ACStrings = CreateDefaultSubobject<UAudioComponent>(TEXT("ACStrings"));
	ACStrings->SetupAttachment(SKViewModel);

	ACAnouncer = CreateDefaultSubobject<UAudioComponent>(TEXT("ACAnnouncer"));
	ACAnouncer->SetupAttachment(RootComponent);

	ACHurtSound = CreateDefaultSubobject<UAudioComponent>(TEXT("ACHurtSound"));
	ACHurtSound->SetupAttachment(SKViewModel);
	
	EnemyTrackName = "Enemy_stack";
	StringsTrackName = "Self_stack";
	
	//Replication
	bReplicates = true;
}

// Called when the game starts or when spawned
void APlayerBehaviour::BeginPlay()
{
	Super::BeginPlay();
	
	SKMTPWeapon->AttachToComponent(SKMTPCharacter, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName(TEXT("TPWeaponSocket")));

	vDefaultPlayerScale = this->GetActorRelativeScale3D();
	iMaxOvercharges = fMovementSpeedOnStage.Num() - 1;

	
	DynamicMat1=UMaterialInstanceDynamic::Create(SKMTPCharacter->GetMaterial(1), this);
	SKMTPCharacter->SetMaterial(1, DynamicMat1);
	DynamicMat5=UMaterialInstanceDynamic::Create(SKMTPCharacter->GetMaterial(5), this);
	SKMTPCharacter->SetMaterial(5, DynamicMat5);
	DynamicMat10=UMaterialInstanceDynamic::Create(SKMTPCharacter->GetMaterial(10), this);
	SKMTPCharacter->SetMaterial(10, DynamicMat10);
	DynamicMatView5=UMaterialInstanceDynamic::Create(SKViewModel->GetMaterial(5), this);
	SKViewModel->SetMaterial(5, DynamicMatView5);
	DynamicMatView0=UMaterialInstanceDynamic::Create(SKViewModel->GetMaterial(0), this);
	SKViewModel->SetMaterial(0, DynamicMatView0);
	DynamicMatView9=UMaterialInstanceDynamic::Create(SKViewModel->GetMaterial(9), this);
	SKViewModel->SetMaterial(9, DynamicMatView9);
	

	if (!HasAuthority())
	{
		Server_Rep_OverchargeStage(iOverchargeStage);
	}
	
	ApplyOvercharge();
	AddWidgetsToViewport();
	//HideWeapon(true);

	ACBase->Play();
	ACBeat->Play();
	ACStrings->Play();
}

// Called every frame
void APlayerBehaviour::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Stop the interpolation if the floats are nearly equal
	if (!UKismetMathLibrary::NearlyEqual_FloatFloat(PlayerCamera->FieldOfView, fFOVToInterpTo, 0.01))
	{
		SetCameraFOV(iOverchargeStage);
		SetPlayerScale(iOverchargeStage);
	}
}

#pragma region PlayerInputs

// Called to bind functionality to input
void APlayerBehaviour::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerBehaviour::MoveForward);
	PlayerInputComponent->BindAxis("MoveSideways", this, &APlayerBehaviour::MoveSideways);
	PlayerInputComponent->BindAction("Overcharge", IE_Pressed, this, &APlayerBehaviour::Overcharge);
	PlayerInputComponent->BindAction("Discharge", IE_Pressed, this, &APlayerBehaviour::Discharge);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerBehaviour::HorizontalMouseCameraMovement);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerBehaviour::VerticalMouseCameraMovement);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerBehaviour::HorizontalJoystickCameraMovement);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerBehaviour::VerticalJoystickCameraMovement);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerBehaviour::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APlayerBehaviour::EndJump);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerBehaviour::StartAttack);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerBehaviour::ReloadShotgun);
	PlayerInputComponent->BindAction("Debug", IE_Pressed, this, &APlayerBehaviour::AddDebugInfoWidgetToViewport);

	PlayerInputComponent->BindAction("UsingController", IE_Pressed, this, &APlayerBehaviour::UpdateInputToController);
	PlayerInputComponent->BindAction("UsingKeyboard", IE_Pressed, this, &APlayerBehaviour::UpdateInputToKeyboard);
}


void APlayerBehaviour::ChooseAndPlaySound(int SoundToPlay)
{
	
		ACAnouncer->SetSound(AllAnouncerSounds[SoundToPlay]);
	//	NetMulticast_Rep_Sound(ACAnouncer);
		Server_OnRep_Sound(ACAnouncer, AllAnouncerSounds[SoundToPlay]);
}

void APlayerBehaviour::Client_OnRep_Sound_Implementation(UAudioComponent* SoundToReplicate, USoundBase* Audio)
{
	if (SoundToReplicate)
	{
		ACAnouncer->SetSound(Audio);
		SoundToReplicate->Play();
	}
}

void APlayerBehaviour::Server_OnRep_Sound_Implementation(UAudioComponent* SoundToReplicate,  USoundBase* Audio)
{
	
	Client_OnRep_Sound(SoundToReplicate, Audio);
	//NetMulticast_Rep_Sound(SoundToReplicate);
}



void APlayerBehaviour::DisablePlayerInput()
{
	Server_OnRep_DisableInput();
}

void APlayerBehaviour::EnablePlayerInput()
{
	if (!GetController()) return;
	this->EnableInput(Cast<APlayerController>(this->GetController()));
	this->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void APlayerBehaviour::HideFPModel()
{
	Server_OnRep_HideMesh();
}

void APlayerBehaviour::ShowFPModel()
{
	Server_OnRep_ShowMesh();
}

void APlayerBehaviour::SetIsScouting(bool State)
{
	bIsScouting = State;
	Server_Rep_IsScouting(bIsScouting);
}

#pragma endregion PlayerInputs

#pragma region Widgets

void APlayerBehaviour::SetPlayerSettings(float Sensitivity, bool CameraYInverted)
{
	// fMouseSensitivity = MouseSensitivity;
	// fJoystickSensitivity = JoystickSensitivity;
	// bIsMouseInverted = MouseInverted;
	// bUseAutoReload = AutoReloadWeapon;
}

void APlayerBehaviour::UpdateInputToController()
{
	bIsUsingController = true;
}

void APlayerBehaviour::UpdateInputToKeyboard()
{
	bIsUsingController = false;
}

bool APlayerBehaviour::IsUsingController()
{
	return bIsUsingController;
}

void APlayerBehaviour::AddWidgetsToViewport()
{
	AddTimerWidgetToViewport();
	AddHealthWidgetToViewport();
	AddAmmoWidgetToViewport();
	AddCrosshairToViewport();
	AddChargesWidgetToViewport();
}

void APlayerBehaviour::RemoveWidgetsFromViewport()
{
	if (wReloadingWidget)
	{
		RemoveReloadingWidgetFromViewport();
	}
	if (wEnemyKilledWidget)
	{
		RemovePlayerKilledWidgetFromViewport();
	}
	if (wReloadingWidget)
	{
		RemovePlayerKilledWidgetFromViewport();
	}
	if (wPickupDeliveredWidget)
	{
		RemovePickupDeliveredWidgetFromViewport();
	}
	if (wHoldingPickupWidget)
	{
		RemoveHoldingPickupWidgetFromViewport();
	}
}

void APlayerBehaviour::AddTimerWidgetToViewport()
{
	if (wMatchInfoClass == nullptr)
	{
		return;
	}

	wMatchInfoWidget = CreateWidget<UUserWidget>(GetWorld(), wMatchInfoClass);

	if (wMatchInfoWidget == nullptr)
	{
		return;
	}

	wMatchInfoWidget->AddToViewport();
}

void APlayerBehaviour::AddHealthWidgetToViewport()
{
	if (wHealthClass == nullptr)
	{
		return;
	}

	wHealthWidget = CreateWidget<UUserWidget>(GetWorld(), wHealthClass);

	if (wHealthWidget == nullptr)
	{
		return;
	}

	wHealthWidget->AddToViewport();
}

void APlayerBehaviour::AddAmmoWidgetToViewport()
{
	if (wAmmoClass == nullptr)
	{
		return;
	}

	wAmmoWidget = CreateWidget<UUserWidget>(GetWorld(), wAmmoClass);

	if (wAmmoWidget == nullptr)
	{
		return;
	}

	wAmmoWidget->AddToViewport();
}

void APlayerBehaviour::AddChargesWidgetToViewport()
{
	if (wChargesClass == nullptr)
	{
		return;
	}

	wChargesWidget = CreateWidget<UUserWidget>(GetWorld(), wChargesClass);

	if (wChargesWidget == nullptr)
	{
		return;
	}

	wChargesWidget->AddToViewport();
}

void APlayerBehaviour::AddCrosshairToViewport()
{
	if (wCrosshairClass == nullptr)
	{
		return;
	}

	wCrossHairWidget = CreateWidget<UUserWidget>(GetWorld(), wCrosshairClass);

	if (wCrossHairWidget == nullptr)
	{
		return;
	}

	wCrossHairWidget->AddToViewport();
}

void APlayerBehaviour::PlayReloadingWidgetToViewport()
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

void APlayerBehaviour::RemoveReloadingWidgetFromViewport() const
{
	wReloadingWidget->RemoveFromViewport();
}

void APlayerBehaviour::AddDebugInfoWidgetToViewport()
{
	if (bIsShowingDebug)
	{
		return;
	}
	UUserWidget* wDebugInfo = CreateWidget<UUserWidget>(GetWorld(), wDebugInfoClass);
	wDebugInfo->AddToViewport();

	bIsShowingDebug = true;
}

void APlayerBehaviour::AddPlayerKilledWidgetToViewport()
{
	if (wEnemyKilledClass == nullptr)
	{
		return;
	}

	wEnemyKilledWidget = CreateWidget<UUserWidget>(GetWorld(), wEnemyKilledClass);

	if (wEnemyKilledWidget == nullptr)
	{
		return;
	}

	wEnemyKilledWidget->AddToViewport();

	GetWorld()->GetTimerManager().SetTimer(fthEnemyKillerTimerHandle, this,
	                                       &APlayerBehaviour::RemovePlayerKilledWidgetFromViewport,
	                                       fEnemyKillerWidgetDisplayTime, false);
}

void APlayerBehaviour::RemovePlayerKilledWidgetFromViewport() const
{
	wEnemyKilledWidget->RemoveFromViewport();
}

void APlayerBehaviour::AddPickupDeliveredWidgetToViewport()
{
	if (wPickupDeliveredClass == nullptr)
	{
		return;
	}

	wPickupDeliveredWidget = CreateWidget<UUserWidget>(GetWorld(), wPickupDeliveredClass);

	if (wPickupDeliveredWidget == nullptr)
	{
		return;
	}

	RemoveHoldingPickupWidgetFromViewport();
	
	wPickupDeliveredWidget->AddToViewport();
	
	FTimerHandle fTHPickupDelivered;
	GetWorld()->GetTimerManager().SetTimer(fTHPickupDelivered, this,
									   &APlayerBehaviour::RemovePickupDeliveredWidgetFromViewport,
									   2.0f, false);
}

void APlayerBehaviour::RemovePickupDeliveredWidgetFromViewport() const
{
	wPickupDeliveredWidget->RemoveFromViewport();
}

void APlayerBehaviour::AddHoldingPickupWidgetToViewport()
{
	if (wHoldingPickupClass == nullptr)
	{
		return;
	}

	wHoldingPickupWidget = CreateWidget<UUserWidget>(GetWorld(), wHoldingPickupClass);

	if (wHoldingPickupWidget == nullptr)
	{
		return;
	}

	wHoldingPickupWidget->AddToViewport();
}

void APlayerBehaviour::RemoveHoldingPickupWidgetFromViewport() const
{
	wHoldingPickupWidget->RemoveFromViewport();
}

void APlayerBehaviour::AddWinWidgetToViewport()
{
	Server_Rep_WinScreen();
}

void APlayerBehaviour::Server_Rep_WinScreen_Implementation()
{
	Client_Rep_WinScreen();
}

void APlayerBehaviour::Client_Rep_WinScreen_Implementation()
{
	if (wWinScreenClass == nullptr)
	{
		return;
	}

	wWinWidget = CreateWidget<UUserWidget>(GetWorld(), wWinScreenClass);

	if (wWinWidget == nullptr)
	{
		return;
	}

	wWinWidget->AddToViewport();

	AddStatsWidgetToViewport();
}

void APlayerBehaviour::AddLoseWidgetToViewport()
{
	Server_Rep_LoseScreen();
}

void APlayerBehaviour::Server_Rep_LoseScreen_Implementation()
{
	Client_Rep_LoseScreen();
}

void APlayerBehaviour::Client_Rep_LoseScreen_Implementation()
{

	if (wLoseScreenClass == nullptr)
	{
		return;
	}

	wLoseWidget = CreateWidget<UUserWidget>(GetWorld(), wLoseScreenClass);

	if (wLoseWidget == nullptr)
	{
		return;
	}

	wLoseWidget->AddToViewport();

	AddStatsWidgetToViewport();
}

void APlayerBehaviour::AddTieWidgetToViewport()
{
	Server_Rep_TieScreen();
}

void APlayerBehaviour::Server_Rep_TieScreen_Implementation()
{
	Client_Rep_TieScreen();
}

void APlayerBehaviour::Client_Rep_TieScreen_Implementation()
{
	if (wTieScreenClass == nullptr)
	{
		return;
	}

	wTieWidget = CreateWidget<UUserWidget>(GetWorld(), wTieScreenClass);

	if (wTieWidget == nullptr)
	{
		return;
	}

	wTieWidget->AddToViewport();

	AddStatsWidgetToViewport();
}

void APlayerBehaviour::AddStatsWidgetToViewport()
{
	if (wStatsClass == nullptr)
	{
		return;
	}

	wStatsWidget = CreateWidget<UUserWidget>(GetWorld(), wStatsClass);

	if (wStatsWidget == nullptr)
	{
		return;
	}

	wStatsWidget->AddToViewport();
}

#pragma endregion Widgets

#pragma region PlayerCameraMovement

void APlayerBehaviour::HorizontalMouseCameraMovement(float Value)
{
	if (Value != 0)
	{
		bIsUsingController = false;

		AddControllerYawInput(Value * fCameraSensitivity);
	}
}

void APlayerBehaviour::VerticalMouseCameraMovement(float Value)
{
	if (Value != 0)
	{
		bIsUsingController = false;

		SetPitch();
		
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

void APlayerBehaviour::HorizontalJoystickCameraMovement(float Value)
{
	
	if (Value != 0)
	{
		bIsUsingController = true;

		AddControllerYawInput(Value * fCameraSensitivity);
	}
}

void APlayerBehaviour::VerticalJoystickCameraMovement(float Value)
{
	
	if (Value != 0)
	{
		bIsUsingController = true;

		SetPitch();
		
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

void APlayerBehaviour::StrafeCameraTilt(float Value) const
{
	const FRotator rCurrentRotation = Controller->GetControlRotation();
	const FRotator rTarget = FRotator(Controller->GetControlRotation().Pitch, Controller->GetControlRotation().Yaw,
	                               fMaxCameraStrafeTilt * Value);
	const FRotator rStrafeTilt = FMath::RInterpTo(rCurrentRotation, rTarget, GetWorld()->GetDeltaSeconds(),
	                                           fStrafeTiltInterpolationSpeed);

	Controller->SetControlRotation(rStrafeTilt);
}

void APlayerBehaviour::SetCameraFOV(int Value)
{
	float fStartFOV = PlayerCamera->FieldOfView;
	fFOVToInterpTo  = fDefaultPlayerFOV + (Value * Value);

	float fNewFOV = UKismetMathLibrary::FInterpTo(fStartFOV,fFOVToInterpTo, GetWorld()->DeltaTimeSeconds,10.0f);
	
	PlayerCamera->SetFieldOfView(fNewFOV);
}

void APlayerBehaviour::SetPlayerScale(int Value)
{
	Server_Rep_SetScale(Value);
}

void APlayerBehaviour::SetPitch()
{
		FRotator rNormalizedRotator = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(),GetActorRotation());
		FRotator rInterpRot = UKismetMathLibrary::RInterpTo(FRotator(fPitch,0.0f,0.0f),rNormalizedRotator, GetWorld()->GetDeltaSeconds(),20.0f);

		fPitch = UKismetMathLibrary::ClampAngle(rInterpRot.Pitch, -90.0f, 90.0f);

		if (HasAuthority())
		{
			NetMulticast_Rep_SetPitchOnClients(fPitch);
		}
		if (!HasAuthority())
		{
			Server_Rep_SetPitchOnServer(fPitch);
		}
}


float APlayerBehaviour::GetPitch()
{
	return fPitch;
}


#pragma endregion PlayerCameraMovement

#pragma region PlayerBaseMovement

void APlayerBehaviour::MoveForward(float Value)
{
	if (Value != 0)
	{
		bIsNotMoving = false;
		GetWorld()->GetTimerManager().ClearTimer(fthOnPlayerStop);
		//Find out which way is forward and record that the player wants to move that way
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, Value);
	}
	if(Value == 0 && !bIsNotMoving)
	{
		bIsNotMoving = true;
		GetWorld()->GetTimerManager().SetTimer(fthOnPlayerStop, this, &APlayerBehaviour::Discharge, 3.0f, false, 3.0f);
	}

	
}

void APlayerBehaviour::MoveSideways(float Value)
{
	fSidewaysValue = Value;
	
	if (Value != 0)
	{
		bIsNotMoving = false;
		GetWorld()->GetTimerManager().ClearTimer(fthOnPlayerStop);
		
		//Find out which way is Sideways, and record that the player wants to move that way
		const FVector Direction = GetActorRightVector();
		AddMovementInput(Direction, Value);
		StrafeCameraTilt(Value);
	}
	if(Value == 0 && !bIsNotMoving)
	{
		bIsNotMoving = true;
		GetWorld()->GetTimerManager().SetTimer(fthOnPlayerStop, this, &APlayerBehaviour::Discharge, 3.0f, false, 3.0f);
	}
}

void APlayerBehaviour::StartJump()
{

		bPressedJump = true;		
		CheckForOverchargeStage();
		JumpingFeedback();
		CheckIfPlayerIsLanded();
		GetWorld()->GetTimerManager().SetTimer(fthCheckForGround, this, &APlayerBehaviour::CheckIfPlayerIsLanded, 0.01f, true, 0.01f);
}

void APlayerBehaviour::EndJump()
{
	bPressedJump = false;
}


void APlayerBehaviour::CheckIfPlayerIsLanded()
{

	if (!GetCharacterMovement()->IsFalling())
	{
		GetWorld()->GetTimerManager().ClearTimer(fthCheckForGround);
		LandingFeedback();
		ResetOverchargeValues();
	}
}

#pragma endregion PlayerBaseMovement

#pragma region Overcharge

void APlayerBehaviour::Overcharge()
{
	if (iOverchargeConsumables == 0)
	{
		return;	
	}
	
	if (iOverchargeStage >= iMaxOvercharges)
	{
		return;
	}
	
	--iOverchargeConsumables;
	
	if (!HasAuthority())
	{
		++iOverchargeStage;

		if (iOverchargeStage == iMaxOvercharges)
		{
			APS_PlayerStatistics* MyPlayerState = GetController()->GetPlayerState<APS_PlayerStatistics>();
			MyPlayerState->AddOvercharges();
		}

		
		
		SetCameraFOV(iOverchargeStage);
		SetPlayerScale(iOverchargeStage);
		Server_Rep_OverchargeStage(iOverchargeStage);
	}

	if (HasAuthority())
	{
		if (iOverchargeStage == iMaxOvercharges)
		{
			APS_PlayerStatistics* MyPlayerState = GetController()->GetPlayerState<APS_PlayerStatistics>();
			MyPlayerState->AddOvercharges();
		}
		
		++iOverchargeStage;
		SetCameraFOV(iOverchargeStage);
		SetPlayerScale(iOverchargeStage);
		Server_Rep_OverchargeStage(iOverchargeStage); 
		//AGM_Arena* ArenaGameMode = (AGM_Arena*)GetWorld()->GetAuthGameMode();
		//ArenaGameMode->UpdateMusicFromPlayers(this->GetController(), iOverchargeConsumables);
	}

	UpdateMusicOnOvercharge();

	ApplyOvercharge();
}

int APlayerBehaviour::OverchargeValueToUI()
{
	return iOverchargeStage;
}

float APlayerBehaviour::MovementSpeedValueToUI()
{
	return fMovementSpeedOnStage[iOverchargeStage];
}

int APlayerBehaviour::CurrentHealthToUI()
{
	return iCurrentHealth;
}

int APlayerBehaviour::CurrentChargesToUI()
{
	return iOverchargeConsumables;
}

int APlayerBehaviour::AmmoToUI()
{
	return iCurrentAmmo;
}

void APlayerBehaviour::SetOvercharge(int iOverchargeValue)
{
	iOverchargeStage = iOverchargeValue;
	
	Server_OnRep_BodyMaterial();
	
	UE_LOG(LogTemp, Warning, TEXT("//////////////////////////////////////////////"));
	UE_LOG(LogTemp, Warning, TEXT("MY OVERCHARGE STAGE: %d"), iOverchargeStage);
	
	
}

void APlayerBehaviour::Discharge()
{
	if (iOverchargeStage == 0)
	{
		return;
	}
	if (!HasAuthority())
	{
		--iOverchargeStage;
		SetCameraFOV(iOverchargeStage);
		SetPlayerScale(iOverchargeStage);
		Server_Rep_OverchargeStage(iOverchargeStage);
	}

	if (HasAuthority())
	{
		--iOverchargeStage;
		SetCameraFOV(iOverchargeStage);
		SetPlayerScale(iOverchargeStage);
		Server_Rep_OverchargeStage(iOverchargeStage);
	}

	UpdateMusicOnDischarge();
	
	ApplyOvercharge();
}

void APlayerBehaviour::ResetOverchargeValues()
{
	ApplyOvercharge();
}

void APlayerBehaviour::ApplyOvercharge()
{
	GetCharacterMovement()->MaxWalkSpeed = fMovementSpeedOnStage[iOverchargeStage];

	if (iOverchargeStage >= 5)
	{
		GetWorld()->GetTimerManager().SetTimer(fthOnPlayerOvercharge,this,&APlayerBehaviour::Discharge, 10.0f, false, 10.0f);
	}
	
	if (!HasAuthority())
	{
		Server_Rep_MovementSpeed(fMovementSpeedOnStage[iOverchargeStage]);
	}
}

void APlayerBehaviour::CheckForOverchargeStage()
{
	GetCharacterMovement()->MaxWalkSpeed = fJumpDistanceOnStage[iOverchargeStage];

	if (!HasAuthority())
	{
		Server_Rep_JumpDistance(fJumpDistanceOnStage[iOverchargeStage]);
	}
}

#pragma endregion Overcharge

#pragma region PlayerShooting

void APlayerBehaviour::StartAttack()
{
	/** Auto Reload */
	if (iCurrentAmmo == 0 && !bIsReloading && !bIsScouting)
	{
		bCanFireShotgun = false;

		ReloadShotgun();
	}
	
	/** Fire */
	if (iCurrentAmmo > 0 && !bIsReloading && !bIsScouting)
	{
		UseShotgun();
	}
}

void APlayerBehaviour::UseShotgun()
{
	if (bCanFireShotgun && !bIsReloading && !bIsScouting)
	{
		ShootingFeedback();
		bCanFireShotgun = false;
		--iCurrentAmmo;
				
		Client_Rep_CurrentAmmo(iCurrentAmmo);
		FireShotgunOnServer();
		
		if (iCurrentAmmo == 0)
			ReloadShotgun();
	}
	else if (!bCanFireShotgun)
	{
		UE_LOG(LogTemp, Warning, TEXT("On Cooldown"));
	}
}

void APlayerBehaviour::FireShotgunOnServer() 
{
	FVector vForwardVector = PlayerCamera->GetForwardVector();
	FVector vUpVector = PlayerCamera->GetUpVector();
	FVector vRightVector = PlayerCamera->GetRightVector();

	//Linetrace from camera not the weapon
	FVector vStart = PlayerCamera->GetComponentLocation();

	FVector vEnd = (vStart + (vForwardVector * 20000.0f));

	int RandSpreadUpPositive;
	int RandSpreadUpNegative;
	int RandSpreadRightPositive;
	int RandSpreadRightNegative;
	
	RandSpreadUpPositive = FMath::RandRange(iMinSlugOpening, iMaxSlugOpening);
	RandSpreadRightNegative = FMath::RandRange(-iMaxSlugOpening, -iMinSlugOpening);
	FVector vSpread1 = (vEnd + vUpVector * RandSpreadUpPositive + vRightVector * RandSpreadRightNegative);

	RandSpreadUpPositive = FMath::RandRange(iMinSlugOpening, iMaxSlugOpening);
	RandSpreadRightPositive = FMath::RandRange(iMinSlugOpening, iMaxSlugOpening);
	FVector vSpread2 = (vEnd + vUpVector * RandSpreadUpPositive + vRightVector * RandSpreadRightPositive);

	RandSpreadUpNegative = FMath::RandRange(-iMaxSlugOpening, -iMinSlugOpening);
	RandSpreadRightPositive = FMath::RandRange(iMinSlugOpening, iMaxSlugOpening);
	FVector vSpread3 = (vEnd + vUpVector * RandSpreadUpNegative + vRightVector * RandSpreadRightPositive);

	RandSpreadUpNegative = FMath::RandRange(-iMaxSlugOpening, -iMinSlugOpening);
	RandSpreadRightNegative = FMath::RandRange(-iMaxSlugOpening, -iMinSlugOpening);
	FVector vSpread4 = (vEnd + vUpVector * RandSpreadUpNegative + vRightVector * RandSpreadRightNegative);

	
	if (HasAuthority())
	{
		FHitResult OutHitMain;
		FHitResult OutHitSpread1;
		FHitResult OutHitSpread2;
		FHitResult OutHitSpread3;
		FHitResult OutHitSpread4;

		FCollisionQueryParams CollisionParams;
				
		GetWorld()->LineTraceSingleByChannel(OutHitMain, vStart, vEnd, ECC_Visibility, CollisionParams);
		GetWorld()->LineTraceSingleByChannel(OutHitSpread1, vStart, vSpread1, ECC_Visibility, CollisionParams);
		GetWorld()->LineTraceSingleByChannel(OutHitSpread2, vStart, vSpread2, ECC_Visibility, CollisionParams);
		GetWorld()->LineTraceSingleByChannel(OutHitSpread3, vStart, vSpread3, ECC_Visibility, CollisionParams);
		GetWorld()->LineTraceSingleByChannel(OutHitSpread4, vStart, vSpread4, ECC_Visibility, CollisionParams);
		
	
		
		//In any case we dont want the player to commit suicide by shooting himself :>
		if (OutHitMain.GetActor() != this && OutHitSpread1.GetActor() != this && OutHitSpread2.GetActor() != this && OutHitSpread3.GetActor() != this && OutHitSpread4.GetActor() != this)
		 {
			if(OutHitMain.GetActor())
				Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitMain.Location, GetControlRotation() * -1);

			if(OutHitSpread1.GetActor())
				Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread1.Location, GetControlRotation() * -1);

			if(OutHitSpread2.GetActor())
				Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread2.Location, GetControlRotation() * -1);

			if(OutHitSpread3.GetActor())
				Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread3.Location, GetControlRotation() * -1);

			if(OutHitSpread4.GetActor())
				Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread4.Location, GetControlRotation() * -1);

			
			APS_PlayerStatistics* MyPlayerState = GetController()->GetPlayerState<APS_PlayerStatistics>();

			MyPlayerState->AddShotsFired();
			if (Cast<APlayerBehaviour>(OutHitMain.GetActor()))
			{
				MyPlayerState->AddShotsHit();
				Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitMain.Location, GetControlRotation() * -1);
				Cast<APlayerBehaviour>(OutHitMain.GetActor())->DamagePlayer(iCenterDamage, this);
			}
			else
			{
				MyPlayerState->AddShotsMissed();
			}

			MyPlayerState->AddShotsFired();

			if (Cast<APlayerBehaviour>(OutHitSpread1.GetActor()))
			{
				MyPlayerState->AddShotsHit();
				Cast<APlayerBehaviour>(OutHitSpread1.GetActor())->DamagePlayer(iSpreadDamage, this);
			}
			else
			{
				MyPlayerState->AddShotsMissed();
			}

			MyPlayerState->AddShotsFired();

			if (Cast<APlayerBehaviour>(OutHitSpread2.GetActor()))
			{
				MyPlayerState->AddShotsHit();
				Cast<APlayerBehaviour>(OutHitSpread2.GetActor())->DamagePlayer(iSpreadDamage, this);
			}
			else
			{
				MyPlayerState->AddShotsMissed();
			}

			MyPlayerState->AddShotsFired();

			if (Cast<APlayerBehaviour>(OutHitSpread3.GetActor()))
			{
				MyPlayerState->AddShotsHit();
				Cast<APlayerBehaviour>(OutHitSpread3.GetActor())->DamagePlayer(iSpreadDamage, this);
			}
			else
			{
				MyPlayerState->AddShotsMissed();
			}

			MyPlayerState->AddShotsFired();

			if (Cast<APlayerBehaviour>(OutHitSpread4.GetActor()))
			{
				MyPlayerState->AddShotsHit();
				Cast<APlayerBehaviour>(OutHitSpread4.GetActor())->DamagePlayer(iSpreadDamage, this);
			}
			else
			{
				MyPlayerState->AddShotsMissed();
			}
		 }		
	}

	if (!HasAuthority())
	{
		Server_Rep_Shooting(vStart, vEnd, vSpread1, vSpread2, vSpread3, vSpread4, this);
	}


	if (bUseAutoReloadByOvercharge && iCurrentAmmo == 0)
	{
		ReloadShotgun();
	}

	GetWorld()->GetTimerManager().SetTimer(fthShotgunTimerHandle, this, &APlayerBehaviour::ResetShotgunFireRate, fShotgunFireRate, false);
}

void APlayerBehaviour::FireShotgunOnClient(FVector Start, FVector End, FVector Spread1, FVector Spread2, FVector Spread3, FVector Spread4, AActor* Shooter)
{
	FHitResult OutHitMain;
	FHitResult OutHitSpread1;
	FHitResult OutHitSpread2;
	FHitResult OutHitSpread3;
	FHitResult OutHitSpread4;

	FCollisionQueryParams CollisionParams;
	
	GetWorld()->LineTraceSingleByChannel(OutHitMain, Start, End, ECC_Visibility, CollisionParams);
	GetWorld()->LineTraceSingleByChannel(OutHitSpread1, Start, Spread1, ECC_Visibility, CollisionParams);
	GetWorld()->LineTraceSingleByChannel(OutHitSpread2, Start, Spread2, ECC_Visibility, CollisionParams);
	GetWorld()->LineTraceSingleByChannel(OutHitSpread3, Start, Spread3, ECC_Visibility, CollisionParams);
	GetWorld()->LineTraceSingleByChannel(OutHitSpread4, Start, Spread4, ECC_Visibility, CollisionParams);


	//In any case we dont want the player to commit suicide by shooting himself :)
	if (OutHitMain.GetActor() != Shooter && OutHitSpread1.GetActor() != Shooter && OutHitSpread2.GetActor() != Shooter
		&& OutHitSpread3.GetActor() != Shooter && OutHitSpread4.GetActor() != Shooter)
	{

		if(OutHitMain.GetActor())
			Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitMain.Location, GetControlRotation() * -1);

		if(OutHitSpread1.GetActor())
			Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread1.Location, GetControlRotation() * -1);

		if(OutHitSpread2.GetActor())
			Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread2.Location, GetControlRotation() * -1);

		if(OutHitSpread3.GetActor())
			Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread3.Location, GetControlRotation() * -1);

		if(OutHitSpread4.GetActor())
			Server_Rep_ParticleAndSound(NSHitSpark, nullptr, OutHitSpread4.Location, GetControlRotation() * -1);

		APS_PlayerStatistics* MyPlayerState = GetController()->GetPlayerState<APS_PlayerStatistics>();

		MyPlayerState->AddShotsFired();

		if (Cast<APlayerBehaviour>(OutHitMain.GetActor()))
		{
			MyPlayerState->AddShotsHit();
			Cast<APlayerBehaviour>(OutHitMain.GetActor())->DamagePlayer(iCenterDamage, Shooter);
			
		}
		else
		{
			MyPlayerState->AddShotsMissed();
		}

		MyPlayerState->AddShotsFired();

		if (Cast<APlayerBehaviour>(OutHitSpread1.GetActor()))
		{
			MyPlayerState->AddShotsHit();
			Cast<APlayerBehaviour>(OutHitSpread1.GetActor())->DamagePlayer(iSpreadDamage, Shooter);
		}
		else
		{
			MyPlayerState->AddShotsMissed();
		}

		MyPlayerState->AddShotsFired();

		if (Cast<APlayerBehaviour>(OutHitSpread2.GetActor()))
		{
			MyPlayerState->AddShotsHit();
			Cast<APlayerBehaviour>(OutHitSpread2.GetActor())->DamagePlayer(iSpreadDamage, Shooter);
		}
		else
		{
			MyPlayerState->AddShotsMissed();
		}

		MyPlayerState->AddShotsFired();

		if (Cast<APlayerBehaviour>(OutHitSpread3.GetActor()))
		{
			MyPlayerState->AddShotsHit();
			Cast<APlayerBehaviour>(OutHitSpread3.GetActor())->DamagePlayer(iSpreadDamage, Shooter);
		}
		else
		{
			MyPlayerState->AddShotsMissed();
		}

		MyPlayerState->AddShotsFired();

		if (Cast<APlayerBehaviour>(OutHitSpread4.GetActor()))
		{
			MyPlayerState->AddShotsHit();
			Cast<APlayerBehaviour>(OutHitSpread4.GetActor())->DamagePlayer(iSpreadDamage, Shooter);
		}
		else
		{
			MyPlayerState->AddShotsMissed();
		}
	}
	
	if (bUseAutoReloadByOvercharge && iCurrentAmmo == 0)
	{
		ReloadShotgun();
	}
}

void APlayerBehaviour::ReloadShotgun()
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
		
		GetWorld()->GetTimerManager().SetTimer(fthShotgunReloadTimerHandle, this, &APlayerBehaviour::FinishedReload, fCurrentShotgunReloadTime, false);
		ReloadFeedback();
		PlayReloadingWidgetToViewport();
	}
}

void APlayerBehaviour::FinishedReload()
{
	if (iCurrentAmmo < iMaxAmmo)
	{
		UE_LOG(LogTemp, Warning, TEXT("FINISHED RELOADING"));

		RemoveReloadingWidgetFromViewport();

		iCurrentAmmo = iMaxAmmo;
		Client_Rep_CurrentAmmo(iCurrentAmmo);

		bIsReloading = false;
		bCanFireShotgun = true;
	}
}

void APlayerBehaviour::ResetShotgunFireRate()
{
	bCanFireShotgun = true;
	GetWorld()->GetTimerManager().ClearTimer(fthShotgunTimerHandle);
}

#pragma endregion PlayerShooting

#pragma region Feedback

void APlayerBehaviour::ShootingFeedback()
{
	/** Play Animation */
	if (FireAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(FireAnimation, 1.5f);
	}

	SpawnMuzzleFlash();
	ACShooting->SetIntParameter("WeaponShotNoise", iOverchargeStage);
	Server_Rep_ParticleAndSound(nullptr, ACShooting, this->GetActorLocation(), this->GetControlRotation());
	
	/** Play Cammera Shake */
	UGameplayStatics::PlayWorldCameraShake(GetWorld(),CSShooting,this->GetActorLocation(),0.0f,200.0f, 1.0f,false);
}

void APlayerBehaviour::JumpingFeedback()
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

void APlayerBehaviour::LandingFeedback()
{
	/** Play Animation */
	if (LandingAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(LandingAnimation, 1.0f);
	}
}

void APlayerBehaviour::ReloadFeedback()
{
	ACReload->SetIntParameter("WeaponReloadNoise", iOverchargeStage);
	Server_Rep_ParticleAndSound(nullptr, ACReload, this->GetActorLocation(), this->GetControlRotation());
	
	if (ReloadAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(ReloadAnimation, 1.2f);
	}	
}

#pragma endregion Feedback

#pragma region Healthsystem

void APlayerBehaviour::DamagePlayer(int iDamageToTake, AActor* Killer)
{
	if (bIsSpawnKillProtected)
	{
		return;
	}

	if (bIsPlayerDead)
	{
		return;
	}

	if (iCurrentHealth > 0)
	{
		iCurrentHealth -= iDamageToTake;

		Client_Rep_CurrentHealth(iCurrentHealth);

		if (iCurrentHealth <= 0)
		{
			MyKiller = Killer; 
			iCurrentHealth = 0;
			Client_Rep_CurrentHealth(iCurrentHealth);

			this->DisablePlayerInput();
			Server_Rep_ParticleAndSound(nullptr, ACHurtSound, this->GetActorLocation(), FRotator::ZeroRotator);
			StartSpawnKillProtection();
			NetMulticast_Rep_AddPlayerKilled();

			bIsPlayerDead = true;
			Server_Rep_IsPlayerDead(bIsPlayerDead);
			
			HideFPModel();

			if (HasAuthority())
			{
				NetMulticast_OnRep_DeathRagdoll();
				AGM_Arena* ArenaGameMode = (AGM_Arena*)GetWorld()->GetAuthGameMode();
				ArenaGameMode->UpdatePlayerScores(Killer);
				ArenaGameMode->UpdateMatchScores();
			}
			
			FTimerHandle RespawnHandle;
			GetWorld()->GetTimerManager().SetTimer(RespawnHandle,this, &APlayerBehaviour::Server_OnRep_Respawn, 2.0f ,false);
		}
	}
}

void APlayerBehaviour::StartSpawnKillProtection()
{
	bIsSpawnKillProtected = true;
	Server_Rep_SpawnKillProtection(bIsSpawnKillProtected);

	GetWorld()->GetTimerManager().SetTimer(fTHSpawnKillProtection, this, &APlayerBehaviour::EndSpawnKillProtection, fSpawnKillProtectionTime, false);
}

void APlayerBehaviour::EndSpawnKillProtection()
{
	bIsSpawnKillProtected = false;
	Server_Rep_SpawnKillProtection(bIsSpawnKillProtected);
}

void APlayerBehaviour::ReturnToMainMenu() const
{
	UGameplayStatics::OpenLevel(GetWorld(), "MainMenu", true);
}


void APlayerBehaviour::UpdateMusicOnOvercharge()
{
	if (iOverchargeStage > 0)
		PreviousOverchargeStage = iOverchargeStage - 1;

	if (iOverchargeStage == 0)
		PreviousOverchargeStage = 0;

	FName SelfActualName = FName(StringsTrackName + FString::FromInt(iOverchargeStage));
	FName SelfPreviousName = FName(StringsTrackName + FString::FromInt(PreviousOverchargeStage));
	FName EnemyActualName = FName(EnemyTrackName + FString::FromInt(iOverchargeStage));
	FName EnemyPreviousName = FName(EnemyTrackName + FString::FromInt(PreviousOverchargeStage));
	
	ACStrings->SetFloatParameter(SelfPreviousName, 0.001f);
	ACBeat->SetFloatParameter(EnemyPreviousName, 0.001f);
	
	ACStrings->SetFloatParameter(SelfActualName, 1.0f);
	ACBeat->SetFloatParameter(EnemyActualName, 1.0f);
}

void APlayerBehaviour::UpdateMusicOnDischarge()
{
	PreviousOverchargeStage = iOverchargeStage + 1;

	FName SelfActualName = FName(StringsTrackName + FString::FromInt(iOverchargeStage));
	FName SelfPreviousName = FName(StringsTrackName + FString::FromInt(PreviousOverchargeStage));
	FName EnemyActualName = FName(EnemyTrackName + FString::FromInt(iOverchargeStage));
	FName EnemyPreviousName = FName(EnemyTrackName + FString::FromInt(PreviousOverchargeStage));
	
	ACStrings->SetFloatParameter(SelfPreviousName, 0.001f);
	ACBeat->SetFloatParameter(EnemyPreviousName, 0.001f);
	
	ACStrings->SetFloatParameter(SelfActualName, 1.0f);
	ACBeat->SetFloatParameter(EnemyActualName, 1.0f);
}

void APlayerBehaviour::Client_OnRep_EnableInput_Implementation()
{
	if (!GetController())
	{
		return;
	}
	this->EnableInput(Cast<APlayerController>(this->GetController()));
	this->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void APlayerBehaviour::Server_OnRep_EnableInput_Implementation()
{
	if (!GetController())
	{
		return;
	}
	this->EnableInput(Cast<APlayerController>(this->GetController()));
	this->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Client_OnRep_EnableInput();
}

void APlayerBehaviour::Server_OnRep_BodyMaterial_Implementation()
{
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
	
	NetMulticast_OnRep_BodyMaterial();
}

void APlayerBehaviour::NetMulticast_OnRep_BodyMaterial_Implementation()
{
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

#pragma endregion Healthsystem

#pragma region Pickups

void APlayerBehaviour::PlayerIsHoldingShard(bool bState, AActor* Player)
{
	if (Player == this)
	{
		//AddHoldingPickupWidgetToViewport();
		
		if (bIsHoldingShard)
		{
			return;
		}
	
		bIsHoldingShard = bState;
		UE_LOG(LogTemp,Warning, TEXT("ISHOLDING: %d"), bIsHoldingShard);
	}	
}

void APlayerBehaviour::PlayerIsNearAltar(bool bState, AActor* Player)
{
	if (Player == this)
	{
		bIsNearAltar = bState;
		bCanInteract = bState;
		
		if (bCanInteract && bIsHoldingShard)
		{
			AddPickupDeliveredWidgetToViewport();
			DeliverShard();
		}
	}
}

void APlayerBehaviour::DeliverShard()
{
	
	if (bIsNearAltar && bIsHoldingShard)
	{
		bCanInteract = false;
		bIsHoldingShard = false;

		++iShardsDelivered;

		Server_Rep_AddShard();
				
		if (iShardsDelivered >= iMaxShardsDelivered)
		{
			iShardsDelivered = 0;
			AddOverchargeConsumable();
		}
	}
	
}

void APlayerBehaviour::AddOverchargeConsumable()
{
	++iOverchargeConsumables;

	if (iOverchargeConsumables>iMaxOverchargeConsumables)
	{
		iOverchargeConsumables = iMaxOverchargeConsumables;
	}
}

#pragma endregion Pickups

#pragma  region Replication

void APlayerBehaviour::Server_Rep_OverchargeStage_Implementation(int iOverchargeStageToUpdateOnServer)
{
	SetOvercharge(iOverchargeStageToUpdateOnServer);
}

void APlayerBehaviour::Server_Rep_MovementSpeed_Implementation(float fPlayerMovementSpeedToUpdateOnServer)
{
	ApplyOvercharge();
}

void APlayerBehaviour::NetMulticast_OnRep_DeathRagdoll_Implementation()
{
	this->GetCharacterMovement()->DisableMovement();
	this->SKMTPCharacter->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->SKMTPCharacter->SetAllBodiesSimulatePhysics(true);
	this->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerBehaviour::Server_Rep_ControlRotation_Implementation(FRotator rAimOffsetToReplicate)
{
	rAimOffset = rAimOffsetToReplicate;
}

void APlayerBehaviour::Server_Rep_JumpDistance_Implementation(float fJumpDistanceToUpdateOnServer)
{
	CheckForOverchargeStage();
}

void APlayerBehaviour::Server_Rep_Shooting_Implementation(FVector StartPosition, FVector EndPosition, FVector Spread1,
                                                          FVector Spread2, FVector Spread3, FVector Spread4, AActor* Shooter)
{
	DrawDebugLine(GetWorld(), StartPosition, EndPosition, FColor::Orange, false,0.1f);
	DrawDebugLine(GetWorld(), StartPosition, Spread1, FColor::Orange, false,0.1f);
	DrawDebugLine(GetWorld(), StartPosition, Spread2, FColor::Orange, false,0.1f);
	DrawDebugLine(GetWorld(), StartPosition, Spread3, FColor::Orange, false,0.1f);
	DrawDebugLine(GetWorld(), StartPosition, Spread4, FColor::Orange, false,0.1f);
	
	FireShotgunOnClient(StartPosition, EndPosition, Spread1, Spread2, Spread3, Spread4, Shooter);
}

void APlayerBehaviour::NetMulticast_Rep_AddPlayerKilled_Implementation()
{
	AddPlayerKilledWidgetToViewport();
}

void APlayerBehaviour::Server_Rep_IsPlayerDead_Implementation(bool IsPlayerDead)
{
	bIsPlayerDead = IsPlayerDead;
}

void APlayerBehaviour::Server_Rep_SpawnKillProtection_Implementation(bool IsProtected)
{
	bIsSpawnKillProtected = IsProtected;
}

void APlayerBehaviour::Client_Rep_CurrentHealth_Implementation(int PlayerCurrentHealth)
{
	iCurrentHealth = PlayerCurrentHealth;
}

void APlayerBehaviour::Client_Rep_CurrentAmmo_Implementation(int PlayerCurrentAmmo)
{
	iCurrentAmmo = PlayerCurrentAmmo;
}

void APlayerBehaviour::Server_Rep_PlayerPickup_Implementation(bool bState, AActor* Player)
{
	PlayerIsHoldingShard(bState,Player);
	Client_Rep_PlayerPickup(bState,Player);
}

void APlayerBehaviour::Client_Rep_PlayerPickup_Implementation(bool bState, AActor* Player)
{
	PlayerIsHoldingShard(bState,Player);
	AddHoldingPickupWidgetToViewport();
}

void APlayerBehaviour::Server_Rep_PlayerNearAltar_Implementation(bool bState, AActor* Player)
{
	PlayerIsNearAltar(bState,Player);
	Client_Rep_PlayerNearAltar(bState,Player);
}

void APlayerBehaviour::Client_Rep_PlayerNearAltar_Implementation(bool bState, AActor* Player)
{
	PlayerIsNearAltar(bState,Player);
}

void APlayerBehaviour::NetMulticast_Rep_SetPitchOnClients_Implementation(float Pitch)
{
	fPitch = Pitch;
}

void APlayerBehaviour::Server_Rep_SetPitchOnServer_Implementation(float Pitch)
{
	fPitch = Pitch;
}

void APlayerBehaviour::Server_Rep_SetScale_Implementation(int Value)
{
	FVector vStartScale = this->GetActorRelativeScale3D();
	vScaleToInterpTo  = vDefaultPlayerScale + (Value/20.0f);

	FVector vNewScale = UKismetMathLibrary::VInterpTo(vStartScale, vScaleToInterpTo, GetWorld()->DeltaTimeSeconds,20.0f);
	this->SetActorRelativeScale3D(vNewScale);
	NetMulticast_Rep_SetScale(Value);
}

void APlayerBehaviour::NetMulticast_Rep_SetScale_Implementation(int Value)
{
	FVector vStartScale = this->GetActorRelativeScale3D();
	vScaleToInterpTo  = vDefaultPlayerScale + (Value/20.0f);

	FVector vNewScale = UKismetMathLibrary::VInterpTo(vStartScale, vScaleToInterpTo, GetWorld()->DeltaTimeSeconds,20.0f);
	this->SetActorRelativeScale3D(vNewScale);
}

void APlayerBehaviour::Server_OnRep_Respawn_Implementation()
{
	AGM_Arena* ArenaGameMode = (AGM_Arena*)GetWorld()->GetAuthGameMode();
	
	if (!GetController()) return;
	if (!ArenaGameMode) return;
	ArenaGameMode->RunRespawnTimer(MyKiller, this, this->GetController());

}

void APlayerBehaviour::Server_OnRep_DisableInput_Implementation()
{
	if (!GetController()) return;
	DisableInput(Cast<APlayerController>(this->GetController()));
	this->GetCharacterMovement()->SetMovementMode(MOVE_None);
	Client_OnRep_DisableInput();
}

void APlayerBehaviour::Client_OnRep_DisableInput_Implementation()
{
	if (!GetController()) return;
	this->DisableInput(Cast<APlayerController>(this->GetController()));
	this->GetCharacterMovement()->SetMovementMode(MOVE_None);
}

void APlayerBehaviour::Server_Rep_IsScouting_Implementation(bool State)
{
	bIsScouting = State;
	Client_Rep_IsScouting(bIsScouting);
}

void APlayerBehaviour::Client_Rep_IsScouting_Implementation(bool State)
{
	bIsScouting = State;
}

void APlayerBehaviour::Server_Rep_AddShard_Implementation()
{
	APS_PlayerStatistics* MyPlayerState = GetController()->GetPlayerState<APS_PlayerStatistics>();
	MyPlayerState->AddShardsDelivered();
}

void APlayerBehaviour::Server_Rep_ParticleAndSound_Implementation(UNiagaraSystem* ParticleToReplicate,
UAudioComponent* SoundToReplicate, FVector InLocation, FRotator InRotation)
{
	NetMulticast_Rep_ParticleAndSound(ParticleToReplicate, SoundToReplicate, InLocation, InRotation);
}

void APlayerBehaviour::NetMulticast_Rep_ParticleAndSound_Implementation(UNiagaraSystem* ParticleToReplicate,
	UAudioComponent* SoundToReplicate, FVector InLocation, FRotator InRotation)
{
	if (SoundToReplicate)
	{
		SoundToReplicate->Play();
	}

	if (ParticleToReplicate)
	{
		//Particle Code
	}
	
}

void APlayerBehaviour::Client_OnRep_HideMesh_Implementation()
{
	SKViewModel->SetHiddenInGame(true);
}

void APlayerBehaviour::Server_OnRep_HideMesh_Implementation()
{
	Client_OnRep_HideMesh();
}

void APlayerBehaviour::Client_OnRep_ShowMesh_Implementation()
{
	SKViewModel->SetHiddenInGame(false);
	if (WeaponPickupAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = SKViewModel->GetAnimInstance();
		AnimInstance->Montage_Play(WeaponPickupAnimation, 1.0f);
	}
}

void APlayerBehaviour::Server_OnRep_ShowMesh_Implementation()
{
	Client_OnRep_ShowMesh();
}

#pragma endregion Replication



