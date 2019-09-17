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
			CurrentDoor = Player->GetDoorInteractingWith()->GetRootComponent()->GetChildComponent(0);
			CurrentDoorknob = Player->GetDoorknob();
			//UE_LOG(LogTemp, Warning, TEXT("%f %f"), CurrentDoorknob->GetComponentLocation().X, Player->GetActorLocation().X);
			if (FVector(CurrentDoorknob->GetComponentLocation() - Player->GetActorLocation()).Size() < 50)
			{
				FRotator DoorRotation = CurrentDoor->GetComponentRotation();
				CurrentDoor->SetWorldRotation(FMath::Lerp(DoorRotation, FRotator(DoorRotation.Pitch, DoorRotation.Yaw + 25, DoorRotation.Roll), DeltaTime));
			}
		}
	}
}

void ADoor::DoDoorPhysics(AActor * CurrentDoor)
{

}

