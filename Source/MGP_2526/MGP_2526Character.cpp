// Copyright Epic Games, Inc. All Rights Reserved.

#include "MGP_2526Character.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MGP_2526.h"
#include "EnhancedInputSubsystems.h"

AMGP_2526Character::AMGP_2526Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AMGP_2526Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
		Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
	
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMGP_2526Character::StopSprint);
		if (!SprintAction)
		{
			UE_LOG(LogTemp, Error, TEXT("SprintAction is NULL"));
		}

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMGP_2526Character::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMGP_2526Character::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AMGP_2526Character::LookInput);

		
	}
	else
	{
		UE_LOG(LogMGP_2526, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

}

void AMGP_2526Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector DetectedWallNormal;
    bool bIsRightWall;

    bool bWallFound = IsWallNearby(DetectedWallNormal, bIsRightWall);

    if (bWallFound && GetCharacterMovement()->IsFalling())
    {
        if (!IsWallRunning)
        {
            StartWallRun(DetectedWallNormal, bIsRightWall);
        }

        UpdateWallRun();
    }
    else if (IsWallRunning)
    {
        StopWallRun();
    }
}


void AMGP_2526Character::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);


	/** if (IsWallNearby()){
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Near Wall with WallRunning Tag"));
	} */


}

void AMGP_2526Character::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AMGP_2526Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AMGP_2526Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AMGP_2526Character::DoJumpStart()
{

	// Wall Jumping Mechanic (Override normal jump if wall running)
	if (IsWallRunning)
    {
        FVector JumpDir = WallNormal + FVector::UpVector;
        LaunchCharacter(JumpDir * 600.0f, true, true);

        StopWallRun();
	}
	else
	{
		// pass Jump to the character
		Jump();
	}
}

void AMGP_2526Character::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}


// Sprinting Mechanic
void AMGP_2526Character::StartSprint(){
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}
void AMGP_2526Character::StopSprint(){
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;	
}


//Old Detection Code
/** bool AMGP_2526Character::IsWallNearbyOld()
{
    FVector Start = GetActorLocation();
    Start.Z += 50.0f; //Getting the starting point for the line trace

    FVector RightEnd = Start + (GetActorRightVector() * WallCheckDistance);
    FVector LeftEnd  = Start - (GetActorRightVector() * WallCheckDistance);

    
	FCollisionQueryParams Params;
    // No longer needed - This was was to make sure the line trace could 
	// detect something other than the player before tags.
	//// edit: We need it.
	Params.AddIgnoredActor(this);

    FHitResult HitRight;
    FHitResult HitLeft;

    bool bHitRight = GetWorld()->LineTraceSingleByChannel(
        HitRight,
        Start,
        RightEnd,
        ECC_Visibility,
        Params
    );

    bool bHitLeft = GetWorld()->LineTraceSingleByChannel(
        HitLeft,
        Start,
        LeftEnd,
        ECC_Visibility,
        Params
    );

    // Check tags on any hit components (Make sure the wall is runnable)
    bool bRightIsWall = bHitRight && HitRight.GetComponent() && HitRight.GetComponent()->ComponentHasTag(WallTag);
    bool bLeftIsWall  = bHitLeft  && HitLeft.GetComponent()  && HitLeft.GetComponent()->ComponentHasTag(WallTag);
    
	return bRightIsWall || bLeftIsWall;
} 
	
End of Old Code*/	

// Wall Running Detection

bool AMGP_2526Character::IsWallNearby(FVector& OutWallNormal, bool& IsRightWall)
{
    FVector Start = GetActorLocation();
    Start.Z += 50.0f;

	//How far we want the detection to reach. 
    FVector RightEnd = Start + (GetActorRightVector() * WallCheckDistance);
    FVector LeftEnd  = Start - (GetActorRightVector() * WallCheckDistance);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); //

    FHitResult Hit;

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, RightEnd, ECC_Visibility))
    {
        if (Hit.ImpactNormal.Z < 0.2f)
        {
            OutWallNormal = Hit.ImpactNormal;
            IsRightWall = true;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Right Side Wall Detected"));

            return true;
        }
    }

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, LeftEnd, ECC_Visibility))
    {
        if (Hit.ImpactNormal.Z < 0.2f)
        {
            OutWallNormal = Hit.ImpactNormal;
            IsRightWall = false;
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Left Side Wall Detected"));
            return true;
        }
    }

    return false;
}

void AMGP_2526Character::UpdateWallRun()
{
    FVector Velocity = WallRunDirection * WallRunSpeed;

    GetCharacterMovement()->Velocity = Velocity;

    GetCharacterMovement()->AddForce(-WallNormal * 200000.0f);
}

//Start Wall Run
void AMGP_2526Character::StartWallRun(const FVector& InWallNormal, bool bIsRightWall)
{
    IsWallRunning = true;
    WallNormal = InWallNormal;
	

    FVector Up = FVector::UpVector;

    WallRunDirection = FVector::CrossProduct(WallNormal, Up);

    // Old Code 
	/* if (!bIsRightWall)
    {
        WallRunDirection *= -1;
    } */

	//Makes sure the player is running in the direction they are facing. 
	float Dot = FVector::DotProduct(WallRunDirection, GetActorForwardVector());
	//Otherwise, swap the direction.
	if (Dot < 0)
	{
    	WallRunDirection *= -1;
	}	

    GetCharacterMovement()->GravityScale = WallRunGravityScale;
}

void AMGP_2526Character::StopWallRun()
{
    IsWallRunning = false;
    GetCharacterMovement()->GravityScale = 1.0f;
}