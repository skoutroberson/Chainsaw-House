// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoopCharacter.h"
#include "Door.generated.h"

UCLASS()
class POOP_API ADoor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	APoopCharacter * Player;
	void CheckDoorAngle(AActor* CurrentDoor);
	int CheckDoorAngleCounter = 0;
	USceneComponent* CurrentDoor = nullptr;
	AActor* CurrentDoorActor = nullptr;
	bool DoorPhysics = false;
	void DoDoorPhysics(USceneComponent* CurrentDoor);
	float DoorStartingAngle;
	float DoorAngle;
	float FrameAngle;
	USceneComponent* DoorComponent;

	void InterpRotate(USceneComponent* CurrentDoor);

	void PlayerInteractingWithDoor(float DeltaTime);

	FVector PlayerLocation = FVector(0, 0, 0);
	FVector LastPlayerLocation = FVector(0, 0, 0);

	UPrimitiveComponent* CurrentDoorknob = nullptr;
	TArray<FOverlapInfo> OutOverlaps;
	
};
