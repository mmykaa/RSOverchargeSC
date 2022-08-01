// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MainMenuPlayerBehaviour.generated.h"

UCLASS()
class PROJECTZAPOWA_API AMainMenuPlayerBehaviour : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainMenuPlayerBehaviour();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


#pragma region UI & Settings

	/** Sets the Mouse Input Sensitivity */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")	float fCameraSensitivity;
	
	/** Checks if the Mouse is Inverted */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")	bool bIsCameraYInverted;

	/** Checks if is using Controller */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")	bool bIsUsingController;

	/** Stores the Reloading widget */
	UPROPERTY()	class UUserWidget* wReloadingWidget;

	/** Store the Reloading UMG UserWidget */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI", Meta = (BlueprintProtected = "true"))
	TSubclassOf<class UUserWidget> wReloadingClass;


	//////////////////////////////////////////////////////////////////////////
	
	/** Set Player Settings */
	UFUNCTION()	void SetPlayerSettings(float Sensitivity, bool CameraYInverted);

	/** Update Input Type to Controller */
	UFUNCTION()	void UpdateInputToController();

	/** Update Input Type to Keyboard */
	UFUNCTION()	void UpdateInputToKeyboard();

	/** Update Input Type In UI InputPromp */
	UFUNCTION(BlueprintCallable)bool IsUsingController();

	/** Add Reloading Widget to the player Viewport */
	UFUNCTION()	void PlayReloadingWidgetToViewport();

	/** Remove Reloading Widget from the player Viewport */
	UFUNCTION()	void RemoveReloadingWidgetFromViewport() const;
	
	/** Displays the Ammo UI */
	UFUNCTION(BlueprintCallable) int AmmoToUI();

#pragma endregion UI  & Settings

#pragma region PlayerSettings

	/** Sets the CapsuleSize in the X axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision") float fCapsuleSizeX;

	/** Sets the CapsuleSize in the Y axis */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision") float fCapsuleSizeY;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PlayerMesh") USkeletalMeshComponent* SKMTPCharacter;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PlayerMesh") UStaticMeshComponent* SKMTPWeapon;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlayerMesh", meta=(AllowPrivateAccess = "true")) USkeletalMeshComponent* SKViewModel;

#pragma endregion PlayerSettings

#pragma region Feedback

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* FireAnimation;

	/** AnimMontage to play each time we Reload */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* ReloadAnimation;
	
	/** AnimMontage to play each time we Jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* JumpAnimation;
    	
	/** Stores the Camera Shake When Shooting */
	UPROPERTY(EditAnywhere)	TSubclassOf<UCameraShakeBase> CSShooting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX") UNiagaraSystem* NSMuzzleFlash;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX") UNiagaraSystem* NSHitSpark;
	

	/** Stores the Shooting Sounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) TArray<USoundBase*> ShootingSounds;

	/** Run feedback when shooting */
	UFUNCTION() void ShootingFeedback();
    
	/** Run feedback when Jumping */
	UFUNCTION() void JumpingFeedback();

	UFUNCTION(BlueprintImplementableEvent) void SpawnMuzzleFlash();

	/** Stores the reload audio component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UAudioComponent* ACReload;

	/** Run feedback when reloading */
	UFUNCTION() void ReloadFeedback();

	UPROPERTY() UMaterialInstanceDynamic* DynamicMat1;
	UPROPERTY() UMaterialInstanceDynamic* DynamicMat5;
	UPROPERTY() UMaterialInstanceDynamic* DynamicMat10;
	UPROPERTY() UMaterialInstanceDynamic* DynamicMatView5;
	UPROPERTY() UMaterialInstanceDynamic* DynamicMatView0;
	UPROPERTY() UMaterialInstanceDynamic* DynamicMatView9;

	void ParticleAndSound(UNiagaraSystem* ParticleToReplicate, USoundBase* SoundToReplicate, FVector InLocation, FRotator InRotation);
   

#pragma endregion Feedback

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

	/** Number of Max Overcharges that the player can have */
	UPROPERTY()	int iMaxOvercharges;

	/** The Current Overcharge stage that the player is in */
	UPROPERTY()	int iOverchargeStage;


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

	/** Checks if the player has Landed */
	UFUNCTION()	void CheckIfPlayerIsLanded();

	/** Reset to the Overcharge Value in movement speed upon landing */
	UFUNCTION()	void ResetOverchargeValues();
	
	/** Checks for the current Overcharge stage */
	UFUNCTION()	void CheckForOverchargeStage();

	/** Apply a Z Impulse in the Player */
	UFUNCTION(BlueprintCallable) void StartJump();

	/** The player cannot jump once again if he is in mid air */
	UFUNCTION()	void EndJump();

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

	/** MainMenu Camera To Travel */
	UPROPERTY()	AActor* MainMenuCameraToTravel;

	/** Is the player currently using the main menu? */
	UPROPERTY()	bool bIsInMainMenu;

	/** Can the player use the main menu? */
	UPROPERTY()	bool bCanUseMainMenu;

	/** MainMenu Camera*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TSubclassOf<AActor> MainMenuCamera;

	/** MainMenu Camera To Zero Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraZero;

	/** MainMenu Camera To One Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraOne;

	/** MainMenu Camera To Two Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraTwo;

	/** MainMenu Camera Three To Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraThree;

	/** MainMenu Camera Four To Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraFour;

	/** MainMenu Camera Five To Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraFive;

	/** MainMenu Camera Six To Travel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* MMCameraSix;

	
	//////////////////////////////////////////////////////////////////////////

	
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

	/** Called when the player begins Interacting with UI */
	UFUNCTION(BlueprintCallable) void PlayerIsUsingUI();

	/** Called when the player begins Interacting with UI */
	UFUNCTION(BlueprintCallable) void PlayerIsNotUsingUI();

	/** Switch to First Person Camera, Called in Main Menu Level */
	UFUNCTION(BlueprintCallable) void SwitchToFirstPersonCamera();

	/** Switch to Main Menu Camera, Called in Main Menu Level */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraZero();

	/** Switch to Main Menu Camera, Called in Main Menu Widget */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraOne();

	/** Switch to Main Menu Camera, Called in Main Menu Widget */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraTwo();

	/** Switch to Main Menu Camera, Called in Main Menu Widget */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraThree();

	/** Switch to Main Menu Camera, Called in Main Menu Widget */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraFour();

	/** Switch to Main Menu Camera, Called in Main Menu Widget */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraFive();

	/** Switch to Main Menu Camera, Called in Main Menu Widget */
	UFUNCTION(BlueprintCallable) void SwitchToMainMenuCameraSix();

#pragma endregion Cameras
	
#pragma region PlayerShooting

	/** . */
	UPROPERTY(EditAnywhere)	USceneComponent* ShootingPoint;

	/** . */
	UPROPERTY()	int iCurrentAmmo;

	/** . */
	UPROPERTY()	int iMaxAmmo;

	/** . */
	UPROPERTY()	float fShotgunFireRate;

	/** . */
	UPROPERTY()	FTimerHandle fthShotgunTimerHandle;

	/** . */
	UPROPERTY()	float fDefaultShotgunReloadTime;

	/** . */
	UPROPERTY()	float fCurrentShotgunReloadTime;

	/** . */
	UPROPERTY()	int iMaxSlugOpening;
	
	/** . */
	UPROPERTY()	int iMinSlugOpening;
	
	/** . */
	UPROPERTY()	bool bUseAutoReloadByOvercharge;

	/** . */
	UPROPERTY()	bool bIsReloading;

	/** . */
	UPROPERTY()	FTimerHandle fthShotgunReloadTimerHandle;

	/** . */
	UPROPERTY()	bool bCanFireShotgun;

	/** . */
	UPROPERTY()	int iCenterDamage;

	/** . */
	UPROPERTY()	int iSpreadDamage;


	//////////////////////////////////////////////////////////////////////////


	/** . */
	UFUNCTION()	void StartAttack();

	/** . */
	UFUNCTION()	void UseShotgun();

	/** . */
	UFUNCTION()	void FireShotgun();
	
	/** . */
	UFUNCTION()	void ReloadShotgun();

	/** . */
	UFUNCTION()	void FinishedReload();
	
	/** . */
	UFUNCTION()	void ResetShotgunFireRate();


#pragma endregion PlayerShooting


};