// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Sound/SoundCue.h"
#include "PlayerBehaviour.generated.h"

class UCameraShakeBase;
UCLASS()
class PROJECTZAPOWA_API APlayerBehaviour : public ACharacter
{
	GENERATED_BODY()


private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Sets default values for this character's properties
	APlayerBehaviour();


#pragma region Player

	/** Sets the CapsuleSize in the X axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision") float fCapsuleSizeX;

	/** Sets the CapsuleSize in the Y axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision") float fCapsuleSizeY;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PlayerMesh") USkeletalMeshComponent* SKMTPCharacter;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PlayerMesh") UStaticMeshComponent* SKMTPWeapon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerMesh")	 USkeletalMeshComponent* SKViewModel;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<USoundBase*> AllAnouncerSounds;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACAnouncer;
	
	UFUNCTION() void ChooseAndPlaySound(int SoundToPlay);

	void DisablePlayerInput();
	void EnablePlayerInput();


#pragma endregion Player

#pragma region UI & Settings

	/** Sets the Mouse Input Sensitivity */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")	float fCameraSensitivity;
	
	/** Checks if the Mouse is Inverted */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")	bool bIsCameraYInverted;

	/** Checks if is using Controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")	bool bIsUsingController;

	/** Store the MatchInfo UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wMatchInfoClass;

	/** Store the Health UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wHealthClass;

	/** Store the Ammo UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wAmmoClass;

	/** Store the Charges UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wChargesClass;

	/** Store the Cross-hair UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wCrosshairClass;

	/** Store the Reloading UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wReloadingClass;

	/** Store the EnemyKilled UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wEnemyKilledClass;

	/** Store the Debug UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wDebugInfoClass;

	/** Store the Interact UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wPickupDeliveredClass;

	/** Store the Interact UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wHoldingPickupClass;

	/** Store the Win UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wWinScreenClass;

	/** Store the Lose UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wLoseScreenClass;

	/** Store the Tie UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wTieScreenClass;

	/** Store the Stats UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wStatsClass;
	
	/** Stores the MatchInfo widget */
	UPROPERTY()	class UUserWidget* wMatchInfoWidget;

	/** Stores the Health widget */
	UPROPERTY()	class UUserWidget* wHealthWidget;
	
	/** Stores the Charges widget */
	UPROPERTY()	class UUserWidget* wChargesWidget;

	/** Stores the Ammo widget */
	UPROPERTY()	class UUserWidget* wAmmoWidget;

	/** Stores the Cross-hair widget */
	UPROPERTY()	class UUserWidget* wCrossHairWidget;

	/** Stores the Reloading widget */
	UPROPERTY()	class UUserWidget* wReloadingWidget;

	/** Stores the EnemyKilled widget */
	UPROPERTY()	class UUserWidget* wEnemyKilledWidget;

	/** Stores the Delivered widget */
	UPROPERTY()	class UUserWidget* wPickupDeliveredWidget;
	
	/** Stores the Holding widget */
	UPROPERTY()	class UUserWidget* wHoldingPickupWidget;

	/** Stores the Win widget */
	UPROPERTY()	class UUserWidget* wWinWidget;
	
	/** Stores the Lose widget */
	UPROPERTY()	class UUserWidget* wLoseWidget;
	
	/** Stores the Tie widget */
	UPROPERTY()	class UUserWidget* wTieWidget;

	/** Stores the Stats widget */
	UPROPERTY()	class UUserWidget* wStatsWidget;


	/** Check if is already showing the debug info */
	UPROPERTY()	bool bIsShowingDebug;

	/** Check if is already showing the debug info */
	UPROPERTY()	bool bIsShowingTimer;

	/** Timer that handles the display time of the widget that gives the player feedback On Kill */
	FTimerHandle fthEnemyKillerTimerHandle;

	/** Check if is already showing the debug info */
	UPROPERTY()	float fEnemyKillerWidgetDisplayTime;



	//////////////////////////////////////////////////////////////////////////

	UFUNCTION() void AddWidgetsToViewport();
	UFUNCTION() void RemoveWidgetsFromViewport();

	/** Set Player Settings */
	UFUNCTION()	void SetPlayerSettings(float Sensitivity, bool CameraYInverted);

	/** Update Input Type to Controller */
	UFUNCTION()	void UpdateInputToController();

	/** Update Input Type to Keyboard */
	UFUNCTION()	void UpdateInputToKeyboard();

	/** Update Input Type In UI InputPromp */
	UFUNCTION(BlueprintCallable)bool IsUsingController();


	
	/** Add Timer Widget to the player Viewport */
	UFUNCTION()	void AddTimerWidgetToViewport();

	/** Add Health Widget to the player Viewport */
	UFUNCTION()	void AddHealthWidgetToViewport();

	/** Add Ammo Widget to the player Viewport */
	UFUNCTION()	void AddAmmoWidgetToViewport();

	/** Add Charges Widget to the player Viewport */
	UFUNCTION()	void AddChargesWidgetToViewport();

	/** Add Crosshair Widget to the player Viewport */
	UFUNCTION()	void AddCrosshairToViewport();
	
	/** Add Reloading Widget to the player Viewport */
	UFUNCTION()	void PlayReloadingWidgetToViewport();

	/** Remove Reloading Widget from the player Viewport */
	UFUNCTION()	void RemoveReloadingWidgetFromViewport() const;

	/** Add Debug Widget to the player Viewport */
	UFUNCTION()	void AddDebugInfoWidgetToViewport();

	/** Add EnemyKilled Widget to the player Viewport */
	UFUNCTION()	void AddPlayerKilledWidgetToViewport();

	/** Remove EnemyKilled Widget from the player Viewport */
	UFUNCTION()	void RemovePlayerKilledWidgetFromViewport() const;

	/** Add Interact Widget to the player Viewport */
	UFUNCTION()	void AddPickupDeliveredWidgetToViewport();

	/** Remove Interact Widget from the player Viewport */
	UFUNCTION()	void RemovePickupDeliveredWidgetFromViewport() const;

	/** Add Interact Widget to the player Viewport */
	UFUNCTION()	void AddHoldingPickupWidgetToViewport();

	/** Remove Interact Widget from the player Viewport */
	UFUNCTION()	void RemoveHoldingPickupWidgetFromViewport() const;
	
	/** Add Win Widget to the player Viewport */
	UFUNCTION()	void AddWinWidgetToViewport();

	/** Add Lose Widget to the player Viewport */
	UFUNCTION()	void AddLoseWidgetToViewport();

	/** Add Tie Widget to the player Viewport */
	UFUNCTION()	void AddTieWidgetToViewport();

	/** Add Stats Widget to the player Viewport */
	UFUNCTION()	void AddStatsWidgetToViewport();
	
	
	/** Send Overcharge value to UI */
	UFUNCTION(BlueprintCallable) int OverchargeValueToUI();

	/** Displays the movement speed in UI */
	UFUNCTION(BlueprintCallable) float MovementSpeedValueToUI();
	
	/** Displays the Health UI */
	UFUNCTION(BlueprintCallable) int CurrentHealthToUI();

	/** Displays the Charges UI */
	UFUNCTION(BlueprintCallable) int CurrentChargesToUI();

	/** Displays the Ammo UI */
	UFUNCTION(BlueprintCallable) int AmmoToUI();

#pragma endregion UI  & Settings

#pragma region Movement

	/** Stores the movement speed of the player in all stages of overcharge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerMovement") TArray<float> fMovementSpeedOnStage;

	/** Stores the jumping speed of the player in all stages of overcharge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerMovement") TArray<float> fJumpDistanceOnStage;

	/** Stores the player acceleration of the player movement */
	UPROPERTY()	float fAcceleration;

	/** Stores the breaking factor of the player movement */
	UPROPERTY()	float fBreakingFactor;

	/** Stores the player impulse force when jumping */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PlayerMovement")	float fJumpForce;

	/** Stores the value of the In-Air Movement Control */
	UPROPERTY()	float fAirMovementControl;

	/** Stores the player Gravity */
	UPROPERTY()	float fPlayerGravity;

	/** Stores if the the condition if player can move or not */
	UPROPERTY()	bool bCanPlayerMove;

	/** Value of the time interval that runs to check if the player has landed */
	UPROPERTY()	FTimerHandle fthCheckForGround;

	bool bIsOnGround;

	/** Number of Max Overcharges that the player can have */
	UPROPERTY()	int iMaxOvercharges;

	/** The Current Overcharge stage that the player is in */
	UPROPERTY()	int iOverchargeStage;

	/** Default Player FOV */
	UPROPERTY()	FVector vDefaultPlayerScale;

	UPROPERTY() FVector vScaleToInterpTo;

	UPROPERTY() float fSidewaysValue;

	/** Timer to check if the player is stoped and decrement his stage */
	UPROPERTY()	FTimerHandle fthOnPlayerStop;
	UPROPERTY()	FTimerHandle fthOnPlayerOvercharge;

	//////////////////////////////////////////////////////////////////////////


	/** Moves the Player Forward (x) and Backwards (-x) */
	UFUNCTION()	void MoveForward(float Value);

	/** Moves the Player to Right (x) and to the left (-x) */
	UFUNCTION()	void MoveSideways(float Value);

	/** Increments the Overcharge stage and uses an RPC to replicate the change */
	UFUNCTION()	void Overcharge();

	/** Stores the Overcharge Value */
	UFUNCTION()	void SetOvercharge(int iOverchargeValue);

	/** Decrements the Overcharge stage and uses an RPC to replicate the change */
	UFUNCTION()	void Discharge();

	/** Apply the Overcharge stage in the movement speed and uses an RPC to replicate the change */
	UFUNCTION()	void ApplyOvercharge();

	bool bCanJump;
	
	/** Checks if the player has Landed */
	UFUNCTION()	void CheckIfPlayerIsLanded();

	
	/** Reset to the Overcharge Value in movement speed upon landing */
	UFUNCTION()	void ResetOverchargeValues();

	/** Apply a Z Impulse in the Player */
	UFUNCTION(BlueprintCallable) void StartJump();

	/** Checks for the current Overcharge stage */
	UFUNCTION()	void CheckForOverchargeStage();

	/** The player cannot jump once again if he is in mid air */
	UFUNCTION()	void EndJump();

	UFUNCTION(BlueprintCallable) float GetSidewaysValue() {return fSidewaysValue;}

#pragma endregion Movement

#pragma region Cameras

	/** First Person Camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = true)) UCameraComponent* PlayerCamera;

	/** First Person Camera Height */
	UPROPERTY(EditAnywhere)	float fPlayerEyeHeight;

	/** First Person Camera Position */
	UPROPERTY()	FVector vFirstPersonCameraPosition;

	/** Max Strafe Angle */
	UPROPERTY()	float fMaxCameraStrafeTilt;

	/** Interpolation Speed while strafing */
	UPROPERTY()	float fStrafeTiltInterpolationSpeed;

	/** Default Player FOV */
	UPROPERTY()	float fDefaultPlayerFOV;

	UPROPERTY() float fFOVToInterpTo;

	UPROPERTY() FRotator rAimOffset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float fPitch;

	UPROPERTY(BlueprintReadWrite) float fMouseX;
	UPROPERTY(BlueprintReadWrite) float fMouseY;
	
	//////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable) float GetMouseX() { return fMouseX; }
	UFUNCTION(BlueprintCallable) float GetMouseY() { return fMouseY; }
	
	/** Camera Mouse Horizontal Movement */
	UFUNCTION()	void HorizontalMouseCameraMovement(float Value);
	
	/** Camera Mouse Vertical Movement */
	UFUNCTION()	void VerticalMouseCameraMovement(float Value);

	/** Camera Joystick Horizontal Movement */
	UFUNCTION()	void HorizontalJoystickCameraMovement(float Value);

	/** Camera Joystick Vertical Movement */
	UFUNCTION()	void VerticalJoystickCameraMovement(float Value);

	/** Camera Strafe Tilt */
	UFUNCTION()	void StrafeCameraTilt(float Value) const;


	/** Set Camera FOV based on Current Overcharge Stage */
	UFUNCTION()	void SetCameraFOV(int Value);
	UFUNCTION() void SetPlayerScale(int Value);
	UFUNCTION() void SetPitch();

	UFUNCTION() void HideFPModel();
	UFUNCTION() void ShowFPModel();

	UFUNCTION(BlueprintCallable) float GetPitch();

#pragma endregion Cameras

#pragma region PlayerShooting
	
	/** Store the shooting point  */
	UPROPERTY(EditAnywhere)	USceneComponent* ShootingPoint;

	/** Stores the current ammo amount */
	UPROPERTY()	int iCurrentAmmo;

	/** Stores the max ammo amount */
	UPROPERTY()	int iMaxAmmo;

	/** Stores the weapon fire rate */
	UPROPERTY()	float fShotgunFireRate;

	/** Fire rate timer handle */
	UPROPERTY()	FTimerHandle fthShotgunTimerHandle;

	/** Stores the default reload time */
	UPROPERTY()	float fDefaultShotgunReloadTime;

	/** Stores the Current reload time */
	UPROPERTY()	float fCurrentShotgunReloadTime;

	/** Stores the Max Spread */
	UPROPERTY()	int iMaxSlugOpening;
	
	/** Stores the Min Spread */
	UPROPERTY()	int iMinSlugOpening;
	
	/** Auto reloading when overcharging */
	UPROPERTY()	bool bUseAutoReloadByOvercharge;

	/** Check if is reloading */
	UPROPERTY()	bool bIsReloading;

	/** Set by the GameMode in the beginning and the end of the Scout Phase */
	UPROPERTY()	bool bIsScouting;
	
	/** Reload Timer Handle */
	UPROPERTY()	FTimerHandle fthShotgunReloadTimerHandle;

	/** Check if can fire the shotgun */
	UPROPERTY()	bool bCanFireShotgun;

	/** Stores the center damage on hit */
	UPROPERTY()	int iCenterDamage;

	/** Stores the spread damage on hit */
	UPROPERTY()	int iSpreadDamage;
	
	//////////////////////////////////////////////////////////////////////////
	
	/** Start the attack action */
	UFUNCTION()	void StartAttack();

	/** Use Shotgun */
	UFUNCTION()	void UseShotgun();

	/** Fires Serverside */
	UFUNCTION()	void FireShotgunOnServer();

	/** Fires Clientside */
	UFUNCTION()	void FireShotgunOnClient(FVector Start, FVector End, FVector Spread1, FVector Spread2, FVector Spread3, FVector Spread4, AActor* Shooter);

	/** Reload the shotgun */
	UFUNCTION()	void ReloadShotgun();

	/** Finishing reload */
	UFUNCTION()	void FinishedReload();

	/** Reset shotgun fire rate */
	UFUNCTION()	void ResetShotgunFireRate();
	
	/** Set by the gamemode */
	UFUNCTION()	void SetIsScouting(bool State);


#pragma endregion PlayerShooting

#pragma region Feedback

	/** AnimMontage to play each time we pickup the weaapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* WeaponPickupAnimation;
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* FireAnimation;
	
	/** AnimMontage to play each time we Reload */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* ReloadAnimation;
	
	/** AnimMontage to play each time we Jump */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* JumpAnimation;
	
	/** AnimMontage to play each time we Jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* LandingAnimation;

	
	/** Stores the Camera Shake When Shooting */
	UPROPERTY(EditAnywhere)	TSubclassOf<UCameraShakeBase> CSShooting;
	
	/** Stores the Shooting Sounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<USoundBase*> ShootingSounds;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACHurtSound;
	UFUNCTION(BlueprintImplementableEvent) void SpawnMuzzleFlash();

	/** Run feedback when shooting */
	UFUNCTION() void ShootingFeedback();
    
    /** Run feedback when Jumping */
    UFUNCTION() void JumpingFeedback();

	UFUNCTION() void LandingFeedback();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX") UNiagaraSystem* NSMuzzleFlash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX") UNiagaraSystem* NSHitSpark;
	
	/** Stores the reload audio component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACReload;

	/** Stores the Shooting audio component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACShooting;
	
	/** Run feedback when reloading */
	UFUNCTION() void ReloadFeedback();

	UMaterialInstanceDynamic* DynamicMat1;
	UMaterialInstanceDynamic* DynamicMat5;
	UMaterialInstanceDynamic* DynamicMat10;
	UMaterialInstanceDynamic* DynamicMatView5;
	UMaterialInstanceDynamic* DynamicMatView0;
	UMaterialInstanceDynamic* DynamicMatView9;

#pragma endregion Feedback

#pragma region Health System

	/** . */
	UPROPERTY()	int iMaxHealth;

	/** . */
	UPROPERTY()	int iCurrentHealth;
	
	/** . */
	UPROPERTY()	bool bIsPlayerDead;

	/** . */
	UPROPERTY()	bool bIsSpawnKillProtected;

	/** . */
	UPROPERTY() FTimerHandle fTHSpawnKillProtection;

	/** . */
	UPROPERTY() float fSpawnKillProtectionTime;

	UPROPERTY() AActor* MyKiller;

	//////////////////////////////////////////////////////////////////////////


	/** Damage Calculations and Check if the player is dead or not */
	UFUNCTION()	void DamagePlayer(int iDamageToTake, AActor* Killer);
	
	/** Starts the Spawn Kill Protection to avoid "SpawnRape" */
	UFUNCTION()	void StartSpawnKillProtection();

	/** Ends the Spawn Kill Protection */
	UFUNCTION()	void EndSpawnKillProtection();

#pragma endregion Health System

#pragma region Shards

	UPROPERTY() bool bIsHoldingShard;

	UPROPERTY() bool bIsNearAltar;

	UPROPERTY() bool bCanInteract;

	UPROPERTY() int iShardsDelivered;

	UPROPERTY() int iMaxShardsDelivered;

	UPROPERTY() int iOverchargeConsumables;

	UPROPERTY() int iMaxOverchargeConsumables;


	
	/////////////////////////////////////////////////////////////////

	
	UFUNCTION() void PlayerIsHoldingShard(bool bState, AActor* Player);
	
	UFUNCTION() void PlayerIsNearAltar(bool bState, AActor* Player);

	UFUNCTION() void DeliverShard();
 
	UFUNCTION() bool CheckIfIsHoldingShard() { return bIsHoldingShard; }
	
	UFUNCTION() void AddOverchargeConsumable();

#pragma endregion Shards

#pragma region Others

	/** Stores the reload audio component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACBase;
	/** Stores the reload audio component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACBeat;
	/** Stores the reload audio component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACStrings;
	
	UFUNCTION(BlueprintCallable) void ReturnToMainMenu() const;
	
	UPROPERTY() int BeatStage;
	UPROPERTY() int PreviousEnemyStage;
	UPROPERTY() int PreviousOverchargeStage;

	UPROPERTY() FString EnemyTrackName;
	UPROPERTY() FString StringsTrackName;
	
	UFUNCTION() void UpdateMusicOnOvercharge();
	UFUNCTION() void UpdateMusicOnDischarge();

	UPROPERTY() bool bIsNotMoving;

#pragma endregion Others

#pragma region Replication

	/** Replicates our OverchargeStage */
	UFUNCTION(Server, Reliable)	void Server_Rep_OverchargeStage(int iOverchargeStageToUpdateOnServer);

	/** Replicates our MovementSpeed */
	UFUNCTION(Server, Reliable)	void Server_Rep_MovementSpeed(float fPlayerMovementSpeedToUpdateOnServer);

	UFUNCTION(Server, Reliable) void Server_Rep_ControlRotation(FRotator rAimOffsetToReplicate);

	/** Replicates our JumpDistance */
	UFUNCTION(Server, Reliable)	void Server_Rep_JumpDistance(float fJumpDistanceToUpdateOnServer);

	/** Replicates the shooting action */
	UFUNCTION(Server, Reliable)	void Server_Rep_Shooting(FVector StartPosition, FVector EndPosition, FVector Spread1, FVector Spread2,
	                         FVector Spread3, FVector Spread4, AActor* Shooter);

	/** Replicates the player eliminated widget */
	UFUNCTION(NetMulticast, Unreliable)	void NetMulticast_Rep_AddPlayerKilled();

	/** Replicates the bIsPLayerDead */
	UFUNCTION(Server, Reliable)	void Server_Rep_IsPlayerDead(bool IsPlayerDead);

	/** Replicates the Ragdoll effect */
	UFUNCTION(NetMulticast, Reliable) void NetMulticast_OnRep_DeathRagdoll();

	/** Replicates the bIsSpawnProtected */
	UFUNCTION(Server, Reliable)	void Server_Rep_SpawnKillProtection(bool IsProtected);

	/** Replicates the player CurrentHealth */
	UFUNCTION(Client, Reliable)	void Client_Rep_CurrentHealth(int PlayerCurrentHealth);

	/** Replicates the player CurrentAmmo */
	UFUNCTION(Client, Reliable)	void Client_Rep_CurrentAmmo(int PlayerCurrentAmmo);

	UFUNCTION(Server, Reliable) void Server_Rep_PlayerPickup(bool bState, AActor * Player);
	UFUNCTION(Client, Reliable) void Client_Rep_PlayerPickup(bool bState, AActor * Player);

	UFUNCTION(Server, Reliable) void Server_Rep_PlayerNearAltar(bool bState, AActor * Player);
	UFUNCTION(Client, Reliable) void Client_Rep_PlayerNearAltar(bool bState, AActor * Player);

	UFUNCTION(Server, Reliable) void Server_Rep_AddShard();

	UFUNCTION(Server, Unreliable) void Server_Rep_SetPitchOnServer(float Pitch);
	UFUNCTION(NetMulticast, Unreliable) void NetMulticast_Rep_SetPitchOnClients(float Pitch);

	UFUNCTION(Server, Unreliable) void Server_Rep_SetScale(int Value);
	UFUNCTION(NetMulticast, Unreliable) void NetMulticast_Rep_SetScale(int Value);

	UFUNCTION(Server,Reliable) void Server_OnRep_Respawn();

	UFUNCTION(Server, Reliable) void Server_Rep_IsScouting(bool State);
	UFUNCTION(Client, Reliable) void Client_Rep_IsScouting(bool State);
	
	UFUNCTION(Server, Reliable) void Server_Rep_WinScreen();
	UFUNCTION(Client, Reliable) void Client_Rep_WinScreen();

	UFUNCTION(Server, Reliable) void Server_Rep_LoseScreen();
	UFUNCTION(Client, Reliable) void Client_Rep_LoseScreen();

	UFUNCTION(Server, Reliable) void Server_Rep_TieScreen();
	UFUNCTION(Client, Reliable) void Client_Rep_TieScreen();

	UFUNCTION(Server, Reliable) void Server_OnRep_HideMesh();
	UFUNCTION(Client, Reliable) void Client_OnRep_HideMesh();

	UFUNCTION(Server, Reliable) void Server_OnRep_ShowMesh();
	UFUNCTION(Client, Reliable) void Client_OnRep_ShowMesh();
	
	UFUNCTION(Server, Unreliable) void Server_OnRep_EnableInput();
	UFUNCTION(Client, Unreliable) void Client_OnRep_EnableInput();
	
	UFUNCTION(Server, Unreliable) void Server_OnRep_DisableInput();
	UFUNCTION(Client, Unreliable) void Client_OnRep_DisableInput();
	
	UFUNCTION(Server, Unreliable) void Server_OnRep_BodyMaterial();
	UFUNCTION(NetMulticast, Unreliable) void NetMulticast_OnRep_BodyMaterial();
	
	UFUNCTION(Server, Unreliable) void Server_Rep_ParticleAndSound(UNiagaraSystem* ParticleToReplicate, UAudioComponent* SoundToReplicate, FVector InLocation, FRotator InRotation);
    UFUNCTION(NetMulticast, Unreliable) void NetMulticast_Rep_ParticleAndSound(UNiagaraSystem* ParticleToReplicate, UAudioComponent* SoundToReplicate, FVector InLocation, FRotator InRotation);

	UFUNCTION(Server, Reliable) void Server_OnRep_Sound(UAudioComponent* SoundToReplicate,  USoundBase* Audio);
	UFUNCTION(Client, Reliable) void Client_OnRep_Sound(UAudioComponent* SoundToReplicate,  USoundBase* Audio);

#pragma endregion Replication
};

