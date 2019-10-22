// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PoopCharacter.h"
#include "PoopProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "Runtime/Engine/Classes/GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Enemy.h"
#include "Runtime/Core/Public/Math/UnrealMathUtility.h"
#include "Runtime/Engine/Classes/Camera/PlayerCameraManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// APoopCharacter

APoopCharacter::APoopCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	GetCharacterMovement()->MaxWalkSpeed *= 0.33f;
	SprintSpeedMultiplier = 3.0f;

	RayCollisionParams.AddIgnoredActor(this);
}

void APoopCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	Player = GetWorld()->GetFirstPlayerController()->GetPawn();
	//FVector Location(61.0f, 976.0f, 300.0f);
	//FRotator Rotation(0.0f, 0.0f, 0.0f);
	//FActorSpawnParameters SpawnInfo;
	//GetWorld()->SpawnActor<AEnemy>(Location, Rotation, SpawnInfo);
	// This line will only work if the player components dont change order!
	//PlayerCam = Player->GetRootComponent()->GetChildComponent(2);

	
	//PlayerController->SetIgnoreLookInput(true);
	//
	//UE_LOG(LogTemp, Warning, TEXT("FUCK"));
	//GetWorld()->GetFirstPlayerController()->GetPawn()->SetActorRotation(FRotator(0, 52, 0));
	//Player->GetRootComponent()->GetChildComponent(2)->SetWorldRotation(FRotator(32, 95, 0));

	//UE_LOG(LogTemp, Warning, TEXT("%f"),this->GetActorRotation().Yaw);

	PlayerYawScale = GetWorld()->GetFirstPlayerController()->InputYawScale;
	PlayerPitchScale = GetWorld()->GetFirstPlayerController()->InputPitchScale;
	UE_LOG(LogTemp, Warning, TEXT("%f %f yo whaddup"), PlayerYawScale, PlayerPitchScale);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APoopCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	//PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APoopCharacter::OnFire);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &APoopCharacter::OnResetVR);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &APoopCharacter::MoveForward);
	PlayerInputComponent->BindAction("MoveForward", IE_Released, this, &APoopCharacter::StoppedMovingForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APoopCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);


	PlayerInputComponent->BindAxis("TurnRate", this, &APoopCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APoopCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APoopCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APoopCharacter::StopSprinting);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APoopCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APoopCharacter::StopInteract);
}

void APoopCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<APoopProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<APoopProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}
	}

}

void APoopCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void APoopCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void APoopCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void APoopCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void APoopCharacter::MoveForward(float Value)
{
	if (IsInteracting == false)
	{
		if (Value != 0.0f)
		{
			LastPlayerLocation = this->GetActorLocation();
			// add movement in that direction
			AddMovementInput(GetActorForwardVector(), Value);
		}
	}
	else if(ShouldOpenDoor == true)
	{
		AddMovementInput(GetActorForwardVector(), Value);
		//Player->GetVelocity().Size();
		USceneComponent CurrentDoor = GetDoorInteractingWith()->GetRootComponent()->GetChildComponent(0);
		//FString DoorName = GetDoorInteractingWith()->GetName();

		//Door = Cast<ADoor>(GetWorld()->GetDefaultSubobjectByName(DoorName));

		//Door->RotateDoor(CurrentDoor, Player->GetVelocity().Size());
	}
}

void APoopCharacter::MoveRight(float Value)
{
	//	Gotta think of a better way to do this...
	if (IsInteracting == false)
	{
		if (Value != 0.0f)
		{
			// add movement in that direction
			AddMovementInput(GetActorRightVector(), Value);
		}
	}
}

void APoopCharacter::Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
}

void APoopCharacter::StopSprinting()
{
	GetCharacterMovement()->MaxWalkSpeed /= SprintSpeedMultiplier;
}

void APoopCharacter::Interact()
{
	FVector ForwardVector = this->GetFirstPersonCameraComponent()->GetForwardVector();
	FVector Start = this->GetFirstPersonCameraComponent()->GetComponentTransform().GetLocation();
	float len = 200.0f;
	FVector End = (ForwardVector * len) + Start;
	//DrawDebugLine(GetWorld(), Start, End, FColor(255, 0, 0), true, 1.0f);

	if (GetWorld()->LineTraceSingleByChannel(HitStruct, Start, End, ECC_WorldDynamic,RayCollisionParams))
	{
		HitActor = HitStruct.GetActor();
		if (HitStruct.GetComponent()->GetName() == "Doorknob")
		{
			Doorknob = HitStruct.GetComponent();
			KnobStartingLoc = Doorknob->GetComponentLocation();
			IsInteracting = true;
			MoveToDoor = true;
			IsInteractingWithDoor = true;
			HitDoor = HitActor->GetRootComponent()->GetChildComponent(0);

			//DoorStartingRotation = HitActor->GetRootComponent()->GetChildComponent(0)->GetComponentRotation();
			//FRotator FrameRotation = HitActor->GetActorRotation();
			
			//FRotator DoorRotation = HitDoor->GetComponentRotation();


			//FRotator DoorRotation = HitActor->GetActorRotation();
			//HitActor->SetActorRotation(FMath::Lerp(DoorRotation,FRotator(DoorRotation.Pitch,DoorRotation.Yaw + 90, DoorRotation.Roll),0.05f));
			//IsInteractingWithDoor = true;
		}
	}
}

void APoopCharacter::StopInteract()
{
	IsInteracting = false;
	if(IsInteractingWithDoor == true)
		IsInteractingWithDoor = false;
	if (MoveToDoor == true)
		MoveToDoor = false;
	if (ShouldOpenDoor == true)
		ShouldOpenDoor = false;

	if (GetWorld()->GetFirstPlayerController()->IsLookInputIgnored())
	{
		GetWorld()->GetFirstPlayerController()->SetIgnoreLookInput(false);
	}
}

void APoopCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APoopCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool APoopCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &APoopCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &APoopCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &APoopCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

void APoopCharacter::MoveDoor(float DeltaTime)
{

	//DoorMovement = FVector::DotProduct((PlayerLocation - LastPlayerLocation) / DeltaTime, HitActor->GetActorForwardVector());

	FRotator DoorRotation = HitDoor->GetComponentRotation();
	FRotator FrameRotation = HitActor->GetActorRotation();
	//UE_LOG(LogTemp, Warning, TEXT("%f"), DeltaTime * doty);
	//HitDoor->SetWorldRotation(FMath::Lerp(DoorRotation,FRotator(DoorRotation.Pitch,DoorRotation.Yaw + doty, DoorRotation.Roll),6 * DeltaTime));
	//HitDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw + doty * DeltaTime * 15, DoorRotation.Roll), 0.05f));
	//HitDoor->AddLocalRotation(FRotator(DoorRotation.Pitch, FMath::FInterpTo(DoorRotation.Yaw, DoorRotation.Yaw + doty * DeltaTime,DeltaTime,0.05f), DoorRotation.Roll));
}

void APoopCharacter::StoppedMovingForward()
{
	UE_LOG(LogTemp, Warning, TEXT("STOP"));
}

void APoopCharacter::InterpToDoor()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	//PlayerController->InputYawScale = 0;
	//PlayerController->InputPitchScale = 0;
	//PlayerController->SetControlRotation(FRotator(0, 83, 32));

	//	If player is at KnobInterpLoc then do the next step in the interacting with door process...
	if (Player->GetActorLocation().Equals(KnobInterpLoc,5.0f))
	{
		MoveToDoor = false;
		ShouldOpenDoor = true;
	}
	else
	{
		//	Disable player look input
		if (!GetWorld()->GetFirstPlayerController()->IsLookInputIgnored())
		{
			GetWorld()->GetFirstPlayerController()->SetIgnoreLookInput(true);
		}
		
		//	Set code to run only on the doorknob the player is interacting with
		if (Doorknob != HitStruct.GetComponent())
			Doorknob = HitStruct.GetComponent();
		
		float DotProd = FVector::DotProduct(Doorknob->GetForwardVector(), Player->GetActorForwardVector());
		

		float FrameRotation = (HitActor->GetActorRotation().Yaw + 180) * (PI / 180);
		float DeltaX = cosf(FrameRotation) * 50;
		float DeltaY = sinf(FrameRotation) * 50;
		float FrameYaw = HitActor->GetActorRotation().Yaw + 180;
		//float PlayerYaw = Player->GetActorRotation().Yaw + 180;
		

		//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), FrameYaw, PlayerYaw, DotProd);

		//UE_LOG(LogTemp, Warning, TEXT("%f --- %f"), Player->GetActorRotation().Yaw + 180, HitActor->GetActorRotation().Yaw + 180);
		

		//PlayerController->SetControlRotation(FMath::Lerp(Player->GetActorRotation(), FRotator(0, 90 - (PlayerYaw - FrameYaw), 0), 0.03f));

		//FVector KnobLocation = Doorknob->GetComponentLocation();
		
		if (DotProd < 0)
		{
			KnobInterpLoc = FVector(KnobStartingLoc.X - DeltaX, KnobStartingLoc.Y - DeltaY, Player->GetActorLocation().Z);
			PlayerController->SetControlRotation(FMath::Lerp(PlayerController->GetControlRotation(), FRotator(0, FrameYaw, 0), 0.06f));
		}
		else if(DotProd > 0)
		{
			KnobInterpLoc = FVector(KnobStartingLoc.X + DeltaX, KnobStartingLoc.Y + DeltaY, Player->GetActorLocation().Z);
			PlayerController->SetControlRotation(FMath::Lerp(PlayerController->GetControlRotation(), FRotator(0, FrameYaw + 180, 0), 0.06f));
		}

		DrawDebugSphere(GetWorld(), KnobStartingLoc, 20, 10, FColor(0, 255, 255), false, 0.1f);
		//FVector InterpLocation = FVector(Doorknob->GetComponentLocation().X,Doorknob->GetComponentLocation().Y,Player->GetActorLocation().Z);
		Player->SetActorLocation(UKismetMathLibrary::VInterpTo(Player->GetActorLocation(), KnobInterpLoc, GetWorld()->GetDeltaSeconds(), 6.0f));
		//float PlayerCamPitch = PlayerCam->GetComponentRotation().Pitch;
		//FRotator DoorCam = FRotator(PlayerCamPitch,0,0);
		//FRotator PlayerCamRotation = PlayerCam->GetComponentRotation();
		//Player->SetActorRotation(FMath::Lerp(Player->GetActorRotation(), FRotator(Player->GetActorRotation().Pitch, 0, 0), 0.5));
		//PlayerCam->SetRelativeRotation(FMath::Lerp(PlayerCamRotation, FRotator(PlayerCamPitch, 0, 0), 1));
		
	}
}

void APoopCharacter::OpenDoor()
{

}

bool APoopCharacter::GetIsInteractingWithDoor()
{
	return IsInteractingWithDoor;
}

AActor * APoopCharacter::GetDoorInteractingWith()
{
	return HitActor;
}

UPrimitiveComponent * APoopCharacter::GetDoorknob()
{
	return Doorknob;
}

void APoopCharacter::Tick(float DeltaTime)
{ 
	Super::Tick(DeltaTime);
	PlayerLocation = Player->GetActorLocation();
	
	if (MoveToDoor == true)
	{
		InterpToDoor();
	}
	else if (ShouldOpenDoor)
	{

	}

	LastPlayerLocation = PlayerLocation;
}
