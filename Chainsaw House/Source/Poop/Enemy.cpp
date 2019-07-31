// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "WorldCollision.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshsmesh(TEXT("/Game/Geometry/Meshes/1M_Cube.1M_Cube"));
	if (meshsmesh.Succeeded())
	{
		mesh->SetStaticMesh(meshsmesh.Object);
		mesh->SetWorldScale3D(FVector(0.6f, 0.6f, 2.0f));
	}

	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Setting Current Location and InterpLocation the same so that the enemy doesnt move at the start.
	CurrentLocation = this->GetActorLocation();
	InterpLocation = CurrentLocation;
	PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();	
	
	nodes = new sNode[nMapWidth * nMapHeight];
	for (int x = 0; x < nMapWidth; x++)
	{
		for (int y = 0; y < nMapHeight; y++)
		{
			int f = y * nMapWidth + x;
			nodes[y * nMapWidth + x].x = x;
			nodes[y * nMapWidth + x].y = y;
			nodes[y * nMapWidth + x].bObstacle = false;
			nodes[y * nMapWidth + x].parent = nullptr;
			nodes[y * nMapWidth + x].bVisited = false;

			float TempZ = 180.0f;
			const FVector & Pos = FVector(x * NodeDist, y * NodeDist, TempZ);
			const FQuat & Qwa = FQuat(0.0f, 0.0f, 0.0f, 0.0f);
			const FVector & BoxHalfExtent = FVector(50.0f, 50.0f, 0.5f);
			const FCollisionShape & Boxy = FCollisionShape::MakeBox(BoxHalfExtent);
			for (TActorIterator<AStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)							// I NEED TO OPTIMIZE THIS SO THAT IT ONLY CHECKS ACTORS CLOSE TO ITS XYZ
			{
				// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
				AStaticMeshActor *SMActor = *ActorItr;
				if (ActorItr->GetStaticMeshComponent()->OverlapComponent(Pos, Qwa, Boxy))
				{
					//DrawDebugSphere(GetWorld(), FVector(x * NodeDist, y * NodeDist, 800), 30, 10, FColor(255, 100, 50), true);
					nodes[y * nMapWidth + x].bObstacle = true;
				}
			}
			//UE_LOG(LogTemp, Warning, TEXT("%d %d %d \n"), x, y, f);
			//UE_LOG(LogTemp, Warning, TEXT("%d %d"), f, y*nMapWidt);
		}
	}
	// Create connections, in this case nodes are on a regular grid
	for (int x = 0; x < nMapWidth; x++)
	{
		for (int y = 0; y < nMapHeight; y++)
		{
			if (y > 0)
			{
				nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x + 0)]);
			}
			if (y < nMapHeight - 1)
			{
				nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x + 0)]);
			}
			if (x > 0)
			{
				nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 0) * nMapWidth + (x - 1)]);
			}
			if (x < nMapWidth - 1)
			{
				nodes[y * nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 0) * nMapWidth + (x + 1)]);
			}

			if (y > 0 && x > 0)
				nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x - 1)]);
			if (y < nMapHeight - 1 && x>0)
				nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x - 1)]);
			if (y > 0 && x < nMapWidth - 1)
				nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x + 1)]);
			if (y < nMapHeight - 1 && x < nMapWidth - 1)
				nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x + 1)]);

		}
	}
				// We can also connect diagonally
				/*if (y>0 && x>0)
					nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x - 1)]);
				if (y<nMapHeight-1 && x>0)
					nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x - 1)]);
				if (y>0 && x<nMapWidth-1)
					nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y - 1) * nMapWidth + (x + 1)]);
				if (y<nMapHeight - 1 && x<nMapWidth-1)
					nodes[y*nMapWidth + x].vecNeighbours.push_back(&nodes[(y + 1) * nMapWidth + (x + 1)]);
				*/

	// Manually position the start and end markers so they are not nullptr
	nodeStart = &nodes[(nMapHeight / 2) * nMapWidth + 1];
	//UE_LOG(LogTemp, Warning, TEXT("%d \n"), nodes[(nMapHeight / 2) * nMapWidth + 1].y);
	DrawDebugSphere(GetWorld(), FVector(nodes[(nMapHeight / 2) * nMapWidth + 1].x * NodeDist, nodes[(nMapHeight / 2) * nMapWidth + 1].y * NodeDist, 210.f), 30, 10, FColor(255, 0, 0), true);
	nodeEnd = &nodes[(nMapHeight / 2) * nMapWidth + nMapWidth - 2];

	//SolveAStar();
}

void AEnemy::SolveAStar()
{
	EnemyPath.Empty();
	// Reset navigation graph - default all node states
	for (int x = 0; x < nMapWidth; x++)
	{
		for (int y = 0; y < nMapHeight; y++)
		{
			nodes[y*nMapWidth + x].bVisited = false;
			nodes[y*nMapWidth + x].fGlobalGoal = INFINITY;
			nodes[y*nMapWidth + x].fLocalGoal = INFINITY;
			nodes[y*nMapWidth + x].parent = nullptr;
		}
	}

	auto distance = [](sNode* a, sNode* b) // For convenience //////////////////////// MIGHT NEED TO MULTIPLY THIS VALUE BY NodeDist
	{
		return sqrtf((a->x - b->x)*(a->x - b->x) + (a->y - b->y)*(a->y - b->y));
	};
	auto heuristic = [distance](sNode* a, sNode* b)	//
	{
		return distance(a, b);
	};

	// Setup starting conditions
	sNode *nodeCurrent = nodeStart;

	nodeStart->fLocalGoal = 0.0f;
	nodeStart->fGlobalGoal = heuristic(nodeStart, nodeEnd);



	// Add start node to not tested list - this will ensure it gets tested
	// as the algorithm progresses, newly discovered nodes get added to the
	// list, and will themselves be tested later
	TArray<sNode*> listNotTestedNodes;
	listNotTestedNodes.Add(nodeStart);
	//listNotTestedNodes.sort([](const sNode* lhs, const sNode* rhs) { return lhs->fGlobalGoal < rhs->fGlobalGoal; });
	// while (listNotTestedNodes.Num() > 0 && nodeCurrent != nodeEnd) <- this code will find the path faster but it may not be the shortest path
	while (listNotTestedNodes.Num() > 0 && nodeCurrent != nodeEnd)
	{
		// Sort Untested nodes by global goal, so lowest is first
		listNotTestedNodes.Sort([](const sNode& lhs, const sNode& rhs){return lhs.fGlobalGoal < rhs.fGlobalGoal;});
		// Front of listNotTestedNodes is potentially the lowest distance node. Our
		// list may also contain nodes that have been visited, so ditch these...
		while (listNotTestedNodes.Num() > 0 && listNotTestedNodes[0]->bVisited == true)
		{
			listNotTestedNodes.RemoveAt(0);
		}
		// ...or abort because there are no more nodes left to test
		if (listNotTestedNodes.Num() == 0)
		{
			break;
		}

		nodeCurrent = listNotTestedNodes[0];
		nodeCurrent->bVisited = true;	// We only explore a node once

		// Check each of this nodes neighbours...
		for (auto nodeNeighbour : nodeCurrent->vecNeighbours)
		{
			// ... and only if the neighbour is not visited and is
			// not an obstacle, add it to the NotTested List
			if (!nodeNeighbour->bVisited && nodeNeighbour->bObstacle == 0)
			{
				listNotTestedNodes.Add(nodeNeighbour);
			}
			// Calculate the neighbours potential lowest parent distance
			float fPossiblyLowerGoal = nodeCurrent->fLocalGoal + distance(nodeCurrent, nodeNeighbour);

			// If choosing to path through this node is a lower distance than what 
			// the neighbour currently has set, update the neighbour to use this node
			// as the path source, and set its distance scores as necessary
			if (fPossiblyLowerGoal < nodeNeighbour->fLocalGoal)
			{
				nodeNeighbour->parent = nodeCurrent;
				nodeNeighbour->fLocalGoal = fPossiblyLowerGoal;

				// The best path length to the neighbour being tested has changed, so
				// update the neighbour's score. The heuristic is used to globally bias
				// the path algorithm, so it knows if its getting better or worse. At some
				// point the algo will realise this path is worse and abandon it, and then go
				// and search along the next best path.
				nodeNeighbour->fGlobalGoal = nodeNeighbour->fLocalGoal + heuristic(nodeNeighbour, nodeEnd);
			}
		}
	}
	FColor DebugCol;
	for (int x = 0; x < nMapWidth; x++)
	{
		for (int y = 0; y < nMapHeight; y++)
		{
			if (nodes[y*nMapWidth + x].bVisited == true)
			{
				DebugCol = FColor(0, 255, 255);
			}
			else if (&nodes[y*nMapWidth + x] == nodeStart)
			{
				DebugCol = FColor(0, 255, 0);
			}
			else if (&nodes[y*nMapWidth + x] == nodeEnd)
			{
				DebugCol = FColor(255, 0, 0);
			}
			else
			{
				DebugCol = FColor(0, 0, 255);
			}
			//DrawDebugSphere(GetWorld(), FVector(x * NodeDist, y * NodeDist, 180.f), 30, 10, DebugCol, true); //////////// For drawing node representations
			
			//////////////////////////////////////////////////////////////////////		FOR DRAWING GRID LINES /////////////////////////////////
			/*for (auto n : nodes[y*nMapWidth + x].vecNeighbours)
			{
				DrawDebugLine(GetWorld(), FVector(nodes[y*nMapWidth + x].x * NodeDist, nodes[y*nMapWidth + x].y * NodeDist, 180.0f), FVector(n->x * NodeDist, n->y * NodeDist, 180.0f), FColor(255, 255, 0), true);
			}*/
		}
	}
	// Draw Path by starting path the end, and following the parent node trail
	// back to the start - the start node will not have a parent path to follow
	if (nodeEnd != nullptr)
	{
		sNode* p = nodeEnd;
		while (p->parent != nullptr)
		{
			//DrawDebugLine(GetWorld(), FVector(p->x * NodeDist, p->y * NodeDist, 182.0f), FVector(p->parent->x * NodeDist, p->parent->y * NodeDist, 182.0f), FColor(255, 0, 0), true,(1.f));
			EnemyPath.Add(p);
			p = p->parent;
		}
	}
	//	Only update InterpLocation if the enemy has a path to the goal node
	if (EnemyPath.Num() > 0)
	{
		UpdateInterpLocation();
	}
}

// Function for updating the next target location for the VInterpTo function

void AEnemy::UpdateInterpLocation()
{
	// Only update InterpLocation if the enemy has a path to follow
	if (EnemyPath.Num() > 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Moving to next node..."));
		sNode* InterpNode = EnemyPath.Pop();
		InterpLocation = FVector(InterpNode->x * NodeDist, InterpNode->y * NodeDist, CurrentLocation.Z);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("No new nodes to move to..."));
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurrentLocation = this->GetActorLocation();
	AStarCallCounter++;
	
	// Check if Enemy has made it to the node its moving to
	if (CurrentLocation.Equals(InterpLocation, 0.1f))
	{
		//Only run UpdateInterpLocation() if there is a path to follow
		if (EnemyPath.Num() > 0)
		{
			// Try to update interp location
			UpdateInterpLocation();
		}
	}

	if (AStarCallCounter == AStarCallTime)
	{
		PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
		//UE_LOG(LogTemp, Warning, TEXT("%f %f"), PlayerLocation.X, PlayerLocation.Y);
		if (PlayerLocation.X / NodeDist > 0 && PlayerLocation.X / NodeDist < nMapWidth)
		{
			if (PlayerLocation.Y / NodeDist > 0 && PlayerLocation.Y / NodeDist < nMapHeight)
			{
				UE_LOG(LogTemp, Warning, TEXT("%d"), EnemyPath.Num());
				PlayerX = PlayerLocation.X / NodeDist;
				PlayerY = PlayerLocation.Y / NodeDist;
				EnemyX = CurrentLocation.X / NodeDist;
				EnemyY = CurrentLocation.Y / NodeDist;
				nodeStart = &nodes[EnemyY * nMapWidth + EnemyX];
				nodeEnd = &nodes[PlayerY * nMapWidth + PlayerX];
				SolveAStar();
			}
		}
		AStarCallCounter = 0;
	}
	CurrentDirection = (InterpLocation - CurrentLocation);
	CurrentDirection.Normalize();
	this->SetActorRotation(FMath::Lerp(GetActorRotation(), CurrentDirection.Rotation(), 0.03f));
	this->SetActorLocation(UKismetMathLibrary::VInterpTo_Constant(CurrentLocation, InterpLocation, DeltaTime, fMovementSpeed));
	//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), InterpLocation.X, InterpLocation.Y, InterpLocation.Z);
}

