// Fill out your copyright notice in the Description page of Project Settings.


#include "Door.h"
#include "PoopCharacter.h"

// Sets default values
ADoor::ADoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADoor::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<APoopCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
}

// Called every frame
void ADoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Player->GetIsInteractingWithDoor())
	{
		PlayerInteractingWithDoor(DeltaTime);
	}

}



void ADoor::DoDoorPhysics(USceneComponent * CurrentDoor)
{

}

void ADoor::InterpRotate(USceneComponent * CurrentDoor)
{
}

void ADoor::PlayerInteractingWithDoor(float DeltaTime)
{
	if (this == Player->GetDoorInteractingWith())
	{
		if (Player->MoveToDoor == false) // bad practice, just doing this to prototype
		{
			PlayerLocation = Player->GetActorLocation();
			CurrentDoorActor = Player->GetDoorInteractingWith();
			CurrentDoor = CurrentDoorActor->GetRootComponent()->GetChildComponent(0);

			//	I FEEL LIKE THIS IS A STUPID STUPID WAY TO GET DOOR KNOB BUT MAYBE NOT.....
			CurrentDoorknob = Player->GetDoorknob();
			//UE_LOG(LogTemp, Warning, TEXT("%f -FUCKFUCKFUCKFUCK-- %f"), CurrentDoor->GetComponentRotation().Yaw + 180, (this->GetActorRotation().Yaw + 180));
			FRotator DoorRotation = CurrentDoor->GetComponentRotation();
			FRotator FrameRotation = this->GetActorRotation();
			float DoorAngle = abs((DoorRotation.Yaw + 180) - (FrameRotation.Yaw + 180));
			if (DoorAngle > 30 && DoorAngle < 270)
			{
				Player->StopInteract();
				DoDoorPhysics(CurrentDoor);
				return;
			}

			float PositionDot = FVector::DotProduct(Player->GetActorForwardVector(), CurrentDoorknob->GetForwardVector());
			float PlayerSpeed = FVector::Dist(PlayerLocation, LastPlayerLocation) * 10;

			if (PositionDot > 0)
				PositionDot = 1;
			else
				PositionDot = -1;

			if (FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size() < 50)	// Push Door
			{	
				//	CALL INTERPROTATE() HERE...
				UE_LOG(LogTemp, Warning, TEXT("%f"), FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size());
				CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw + ((16 * PlayerSpeed) * PositionDot), DoorRotation.Roll), DeltaTime));
			}
			else if (FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size() > 50)		// Pull Door
			{
				//	CALL INTERPROTATE() HERE...
				UE_LOG(LogTemp, Warning, TEXT("%f"), FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size());
				CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw - ((6 * PlayerSpeed) * PositionDot), DoorRotation.Roll), DeltaTime));
			}

			LastPlayerLocation = PlayerLocation;
		}
	}
}

