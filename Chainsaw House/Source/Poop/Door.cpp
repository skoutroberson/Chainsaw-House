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
		if (this == Player->GetDoorInteractingWith())
		{
			if(Player->MoveToDoor == false) // bad practice, just doing this to prototype
			{ 
			PlayerLocation = Player->GetActorLocation();
			CurrentDoor = Player->GetDoorInteractingWith()->GetRootComponent()->GetChildComponent(0);

			//	I FEEL LIKE THIS IS A STUPID STUPID WAY TO GET DOOR KNOB BUT MAYBE NOT.....
			CurrentDoorknob = Player->GetDoorknob();

			//UE_LOG(LogTemp, Warning, TEXT("%f %f"), CurrentDoorknob->GetComponentLocation().X, Player->GetActorLocation().X);
			if (FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size() < 50)
			{
				float PositionDot = FVector::DotProduct(Player->GetActorForwardVector(), CurrentDoorknob->GetForwardVector());
				float PlayerSpeed = FVector::Dist(PlayerLocation, LastPlayerLocation) * 10;
				//UE_LOG(LogTemp, Warning, TEXT("%f poo"), 10 * FVector::Dist(PlayerLocation, LastPlayerLocation));
				FRotator DoorRotation = CurrentDoor->GetComponentRotation();
				if (PositionDot > 0)
				{
					CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw + (12 * PlayerSpeed), DoorRotation.Roll), DeltaTime));
				}
				else 
				{
					CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw - (12 * PlayerSpeed), DoorRotation.Roll), DeltaTime));
				}
				LastPlayerLocation = PlayerLocation;
				
			}
			else if (FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size() > 50)
			{
				float PositionDot = FVector::DotProduct(Player->GetActorForwardVector(), CurrentDoorknob->GetForwardVector());
				float PlayerSpeed = FVector::Dist(PlayerLocation, LastPlayerLocation) * 10;
				//UE_LOG(LogTemp, Warning, TEXT("%f poo"), 10 * FVector::Dist(PlayerLocation, LastPlayerLocation));
				FRotator DoorRotation = CurrentDoor->GetComponentRotation();
				if (PositionDot > 0)
				{
					CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw - (12 * PlayerSpeed), DoorRotation.Roll), DeltaTime));
				}
				else
				{
					CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw + (12 * PlayerSpeed), DoorRotation.Roll), DeltaTime));
				}
				LastPlayerLocation = PlayerLocation;
			}
			LastPlayerLocation = PlayerLocation;
			}
		}
	}
}

void ADoor::DoDoorPhysics(AActor * CurrentDoor)
{

}

