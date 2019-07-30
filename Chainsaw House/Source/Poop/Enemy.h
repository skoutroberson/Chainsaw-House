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
	FVector PlayerLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere)
		class UStaticMeshComponent* mesh;

public:
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

	//	Graph of nodes for A* (representing 2D grid)
	sNode *nodes = nullptr;
	//	Width of A* grid
	int nMapWidth = 100;
	//	Height of A* grid
	int nMapHeight = 100;
	//	Distance nodes are from eachother in A* grid
	int NodeDist = 60;							// 61cm = 2ft
	//	Starting position of enemy for A*
	sNode *nodeStart = nullptr;
	//	Goal position of enemy for A*
	sNode *nodeEnd = nullptr;
	//	Finds A* path
	void SolveAStar();
	//	Stack of nodes that represent the Enemy path
	TArray<sNode*> EnemyPath;
	//	Enemy's movement speed
	float fMovementSpeed = 100.0f;
	//	Enemy's next location to move to
	FVector InterpLocation;
	//	If Enemy has reached InterpLocation, try to update InterpLocation
	void UpdateInterpLocation();
	//	Prototype counter... Need to change
	int MoveToPlayerCounter = 0;
	//	For Integer division of FVector PlayerLocation, Might not need...
	int PlayerX;
	//	For Integer division of FVector PlayerLocation, Might not need...
	int PlayerY;
	//	For Integer division of FVector EnemyLocation, Might not need...
	int EnemyX;
	//	For Integer division of FVector EnemyLocation, Might not need...
	int EnemyY;

	///////////////////////////// FUNCTIONS / VARIABLES / LOGIC I MAY NEED IN THE FUTURE /////////////////////////////
	//		roomNode* root;																	Set this to equal player start room.
	//		(in update function) if (NextRoomToSearch

	sNode* NextRoomToSearch();
};
