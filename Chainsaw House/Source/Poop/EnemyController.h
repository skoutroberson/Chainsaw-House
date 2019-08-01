// Fill out your copyright notice in the Description page of Project Settings.
/*This class will hold AI Logic*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyController.generated.h"

UCLASS()
class POOP_API AEnemyController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyController();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
