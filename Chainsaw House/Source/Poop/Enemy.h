// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <vector>
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Enemy.generated.h"

using namespace std;

UCLASS()
class POOP_API AEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemy();
	FVector Goal;
	FVector CurrentLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* mesh;

private:
	struct sNode 
	{
		bool bObstacle = false;					// Is the node an obstruction?
		bool bVisited = false;					// Have we searched this node before?
		float fGlobalGoal;						// Distance to goal so far
		float fLocalGoal;						// Distance to goal if we took the alternative
		int x;									// Nodes position in 3D space
		int y;
		vector<sNode*> vecNeighbours;			// Connections to neighbours
		sNode* parent;							// Node connecting to this node that offers shortest parent
	};

	sNode *nodes = nullptr;
	int nMapWidth = 32;
	int nMapHeight = 32;
	int NodeDist = 61;							// 61cm = 2ft
	sNode *nodeStart = nullptr;
	sNode *nodeEnd = nullptr;
	void SolveAStar();
	TArray<sNode*> EnemyPath;
	void MoveEnemy(float DeltaTime);
	bool DoneMoving = false;
};
