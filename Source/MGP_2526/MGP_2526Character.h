// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MGP_2526Character.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/** 
* A basic first person character 
*/
UCLASS(abstract)
class AMGP_2526Character : public ACharacter
{
    GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	
	UPROPERTY(EditAnywhere, Category = "Input")	
	class UInputMappingContext* InputMapping;

	/** Sprint input action */
	UPROPERTY(EditAnywhere, Category = "Input")	
	UInputAction* SprintAction;

	void StartSprint(); /* Added In*/
	void StopSprint();
	void UpdateSprint();

	// Wall Run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
	bool IsWallRunning;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Running")
	bool IsTheRightWall;

	void ResetWallRun();

	FVector WallNormal;
	FVector WallRunDirection;

	float MinWallRunSpeed = 600.0f;
	float CurrentWallRunSpeed = 0.0f;
	float WallRunGravityScale = 0.3f;
	float WallRunSpeed = 1200.0f;
	float WallCheckDistance = 100.0f;
	bool CanWallRun = true;
	FTimerHandle WallRunCooldownTimer;

	void StartWallRun(const FVector& InWallNormal, bool IsRightWall);
	void StopWallRun();
	void UpdateWallRun();

	// Sprint Momentum
	float MaxMovementSpeed = 1200.0f;
	float MinMovementSpeed = 550.0f;
	bool isSprinting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SpeedTime = 4.0f;


	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Wall Detection")
	FName WallTag = "Wall";
	
public:
	AMGP_2526Character();
	


protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="WallRunning")
	bool IsWallNearby(FVector &OutWallNormal, bool &IsRightWall);
protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void Tick(float DeltaTime) override;


public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

