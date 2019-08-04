// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include <vector>
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
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
	FVector CurrentDirection;

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
	//	Starting position of enemy for A*
	sNode *nodeStart = nullptr;
	//	Goal position of enemy for A*
	sNode *nodeEnd = nullptr;
	//	Stack of nodes that represent the Enemy path
	TArray<sNode*> EnemyPath;
	//	Width of A* grid
	int nMapWidth = 40;
	//	Height of A* grid
	int nMapHeight = 40;
	//	Distance nodes are from eachother in A* grid. (61cm = 2ft)
	int NodeDist = 80;							
	//	Counter for calling SolveAStar() every n seconds
	int AStarCallCounter = 0;
	//	Variable for checking when AStarCallCounter == AStarCallTime
	int AStarCallTime = 60;
	//	Will need to change the way I do this when I make 2 stories
	int FloorHeight = 180;
	int EnemyHalfWidth = 30;
	//	For Integer division of FVector PlayerLocation, Might not need...
	int PlayerX;
	//	For Integer division of FVector PlayerLocation, Might not need...
	int PlayerY;
	//	For Integer division of FVector EnemyLocation, Might not need...
	int EnemyX;
	//	For Integer division of FVector EnemyLocation, Might not need...
	int EnemyY;
	//	Enemy FOV is 100 degrees
	int FOVHalfangle = 50;
	//	Multiplier to make the FOV triangle (2D view frustum) bigger
	int FOVMultiplier = 1200;
	//	Enemy's movement speed
	float fMovementSpeed = 200.0f;
	//	Enemy's next location to move to
	FVector InterpLocation;
	//	From the POV of the enemy, this is the right point of the FOV triangle (2D view frustum)
	FVector FOVPointR;
	//	From the POV of the enemy, this is the left point of the FOV triangle (2D view frustum)
	FVector FOVPointL;
	//	Height of FOV triangle (2D view frustum), starting value is 2000
	int FOVHeight = 2000;
	//	Check if player location is inside FOV triangle (2D view frustum)
	//	A function to check whether point P(x, y) lies inside the triangle formed
	//	by A(x1, y1), B(x2, y2) and C(x3, y3) 
	bool IsInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y);
	//	Finds A* path
	void SolveAStar();
	//	If Enemy has reached InterpLocation, try to update InterpLocation
	void UpdateInterpLocation();
	//	Returns true if raycast from Start collides with End
	bool ClearPath(FVector Start, FVector End);
	bool InPlayerLOS();
	void ArrivedInterpLoc();
	int DistanceToPlayer();

	FHitResult HitStruct = FHitResult(ForceInit); ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	FCollisionQueryParams ColParams;

	AActor* PlayerActor;

	///////////////////////////// FUNCTIONS / VARIABLES / LOGIC I MAY NEED IN THE FUTURE /////////////////////////////
	//		roomNode* root;																	Set this to equal player start room
	//		(in update function) if (NextRoomToSearch

	sNode* NextRoomToSearch();

};
