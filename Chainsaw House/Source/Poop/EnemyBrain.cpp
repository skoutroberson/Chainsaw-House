// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBrain.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "WorldCollision.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "PoopCharacter.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Physics/GenericPhysicsInterface.h"

// Sets default values for this component's properties
UEnemyBrain::UEnemyBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Enemy = GetOwner();
	RayCollisionParams.AddIgnoredActor(Enemy);
}


// Called when the game starts
void UEnemyBrain::BeginPlay()
{
	Super::BeginPlay();

	EnemyLocation = Enemy->GetActorLocation();
	InterpLocation = EnemyLocation;
	Player = GetWorld()->GetFirstPlayerController()->GetPawn();
	PlayerLocation = Player->GetActorLocation();
	// The more actors I can add to this ignore list the better
	RayCollisionParams.AddIgnoredActor(Player);

	nodes = new ThetaNode[GridWidth * GridHeight];
	for (int x = 0; x < GridWidth; x++)
	{
		for (int y = 0; y < GridHeight; y++)
		{
			int f = y * GridWidth + x;
			nodes[y * GridWidth + x].x = x * NodeDist;
			nodes[y * GridWidth + x].y = y * NodeDist;
			nodes[y * GridWidth + x].bObstacle = false;
			nodes[y * GridWidth + x].parent = nullptr;
			nodes[y * GridWidth + x].bVisited = false;

			float TempZ = 180.0f;
			//////////////////////////////////////////////// all of these variables dont need to be const & Im pretty sure
			const FVector & Pos = FVector(x * NodeDist, y * NodeDist, TempZ);
			const FQuat & Qwa = FQuat(0.0f, 0.0f, 0.0f, 0.0f);
			const FVector & BoxHalfExtent = FVector(EnemyHalfWidth, EnemyHalfWidth, 0.5f);
			const FCollisionShape & Boxy = FCollisionShape::MakeBox(BoxHalfExtent);
			for (TActorIterator<AStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)							// I NEED TO OPTIMIZE THIS SO THAT IT ONLY CHECKS ACTORS CLOSE TO ITS XYZ
			{
				// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
				AStaticMeshActor *SMActor = *ActorItr;
				if (ActorItr->GetStaticMeshComponent()->OverlapComponent(Pos, Qwa, Boxy))
				{
					//DrawDebugSphere(GetWorld(), FVector(x * NodeDist, y * NodeDist, 800), 30, 10, FColor(255, 100, 50), true);
					nodes[y * GridWidth + x].bObstacle = true;
				}
			}
			//UE_LOG(LogTemp, Warning, TEXT("%d %d %d \n"), x, y, f);
			//UE_LOG(LogTemp, Warning, TEXT("%d %d"), f, y*nMapWidt);
		}
	}

	// Create connections, in this case nodes are on a regular grid
	for (int x = 0; x < GridWidth; x++)
	{
		for (int y = 0; y < GridHeight; y++)
		{
			if (y > 0)
			{
				nodes[y * GridWidth + x].NeighbourNodes.push_back(&nodes[(y - 1) * GridWidth + (x + 0)]);
			}
			if (y < GridHeight - 1)
			{
				nodes[y * GridWidth + x].NeighbourNodes.push_back(&nodes[(y + 1) * GridWidth + (x + 0)]);
			}
			if (x > 0)
			{
				nodes[y * GridWidth + x].NeighbourNodes.push_back(&nodes[(y + 0) * GridWidth + (x - 1)]);
			}
			if (x < GridWidth - 1)
			{
				nodes[y * GridWidth + x].NeighbourNodes.push_back(&nodes[(y + 0) * GridWidth + (x + 1)]);
			}

			if (y > 0 && x > 0)
				nodes[y*GridWidth + x].NeighbourNodes.push_back(&nodes[(y - 1) * GridWidth + (x - 1)]);
			if (y < GridHeight - 1 && x>0)
				nodes[y*GridWidth + x].NeighbourNodes.push_back(&nodes[(y + 1) * GridWidth + (x - 1)]);
			if (y > 0 && x < GridWidth - 1)
				nodes[y*GridWidth + x].NeighbourNodes.push_back(&nodes[(y - 1) * GridWidth + (x + 1)]);
			if (y < GridHeight - 1 && x < GridWidth - 1)
				nodes[y*GridWidth + x].NeighbourNodes.push_back(&nodes[(y + 1) * GridWidth + (x + 1)]);

			for (auto n : nodes[y*GridWidth + x].NeighbourNodes)
			{
				DrawDebugLine(GetWorld(), FVector(nodes[y*GridWidth + x].x, nodes[y*GridWidth + x].y, 180.0f), FVector(n->x, n->y, 180.0f), FColor(255, 255, 0), true);
			}

		}
	}

	// Manually position the start and end markers so they are not nullptr
	NodeStart = &nodes[(GridHeight / 2) * GridWidth + 1];
	//UE_LOG(LogTemp, Warning, TEXT("%d \n"), nodes[(GridHeight / 2) * GridWidth + 1].y);
	//DrawDebugSphere(GetWorld(), FVector(nodes[(GridHeight / 2) * GridWidth + 1].x * NodeDist, nodes[(GridHeight / 2) * GridWidth + 1].y * NodeDist, 210.f), 30, 10, FColor(255, 0, 0), true);
	NodeEnd = &nodes[(GridHeight / 2) * GridWidth + GridWidth - 2];

}


// Called every frame
void UEnemyBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AStarCallCounter++;
	EnemyLocation = Enemy->GetActorLocation();
	EnemyDirection = (InterpLocation - EnemyLocation);
	EnemyDirection.Normalize();								// Pretty sure I could optimize the way of getting the rotation that I dont need this calculation every time but not sure...
	Enemy->SetActorRotation(FMath::Lerp(Enemy->GetActorRotation(), EnemyDirection.Rotation(), 0.025f));
	Enemy->SetActorLocation(UKismetMathLibrary::VInterpTo_Constant(EnemyLocation, InterpLocation, DeltaTime, EnemySpeed));
	ArrivedInterpLoc();

	//	Conditional for calling SolveAStar() every n frames
	if (AStarCallCounter == AStarCallTime)
	{
		PlayerLocation = Player->GetActorLocation();

		if (PlayerLocation.X / NodeDist > 0 && PlayerLocation.X / NodeDist < GridWidth)
		{
			if (PlayerLocation.Y / NodeDist > 0 && PlayerLocation.Y / NodeDist < GridHeight)
			{

				//DrawDebugLine(GetWorld(), SightR, FVector(x5, y5, PlayerLocation.Z), FColor(255, 0, 0), false, 0.5f);
				//DrawDebugLine(GetWorld(), SightL, FVector(x4, y4, PlayerLocation.Z), FColor(0, 0, 255), false, 0.5f);

				/*
				if (FVector::Distance(CurrentLocation, PlayerLocation) < FOVHeight)					//	Will need to take into account Z values too!!!
				{
					FOVPointR = FVector(
						CurrentLocation.X + (UKismetMathLibrary::DegCos(GetActorRotation().Yaw + FOVHalfangle) * FOVMultiplier),
						CurrentLocation.Y + (UKismetMathLibrary::DegSin(GetActorRotation().Yaw + FOVHalfangle) * FOVMultiplier),
						CurrentLocation.Z);
					FOVPointL = FVector(
						CurrentLocation.X + (UKismetMathLibrary::DegCos(GetActorRotation().Yaw - FOVHalfangle) * FOVMultiplier),
						CurrentLocation.Y + (UKismetMathLibrary::DegSin(GetActorRotation().Yaw - FOVHalfangle) * FOVMultiplier),
						CurrentLocation.Z);

					DrawDebugSphere(GetWorld(), FOVPointR, 10, 4, FColor(0, 255, 255), true,+1.0f);
					DrawDebugSphere(GetWorld(), FOVPointL, 10, 4, FColor(0, 255, 255), true,+1.0f);
					if (IsInside(CurrentLocation.X,CurrentLocation.Y,FOVPointR.X,FOVPointR.Y,FOVPointL.X,FOVPointL.Y,PlayerLocation.X,PlayerLocation.Y))
					{
						//UE_LOG(LogTemp, Warning, TEXT("%d"), EnemyPath.Num());
					}
				}*/
				PlayerX = roundf(PlayerLocation.X / NodeDist);
				PlayerY = roundf(PlayerLocation.Y / NodeDist);
				EnemyX = roundf(EnemyLocation.X / NodeDist);
				EnemyY = roundf(EnemyLocation.Y / NodeDist);
				NodeStart = &nodes[EnemyY * GridWidth + EnemyX];
				NodeEnd = &nodes[PlayerY * GridWidth + PlayerX];
				SolveThetaStar();									///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				//UE_LOG(LogTemp, Warning, TEXT("%d"), EnemyPath.Num());
				//DrawDebugLine(GetWorld(), FVector(NodeStart->x * NodeDist, NodeStart->y*NodeDist, FloorHeight), FVector(NodeEnd->x*NodeDist, NodeEnd->y*NodeDist, FloorHeight), FColor(0, 255, 0), false, 1.0);
				//FVector Dir = (FVector(NodeEnd->x*NodeDist, NodeEnd->y*NodeDist, FloorHeight) - FVector(NodeStart->x * NodeDist, NodeStart->y*NodeDist, FloorHeight));
				//Dir.RotateAngleAxis(90, FVector(0,0,1));
				//Dir.Normalize();
				//DrawDebugLine(GetWorld(), FVector(Dir * 60), FVector(NodeEnd->x, NodeEnd->y,FloorHeight), FColor(0, 255, 0), false, 1.0);
			}
		}
		AStarCallCounter = 0;
	}

}

void UEnemyBrain::SolveThetaStar()
{
	EnemyPath.Empty();
	// Reset navigation graph - default all node states. I dont need to do this every single time, it helps for debugging but I should change this once this is locked down.
	for (int x = 0; x < GridWidth; x++)
	{
		for (int y = 0; y < GridHeight; y++)
		{
			nodes[y*GridWidth + x].bVisited = false;
			nodes[y*GridWidth + x].fGlobalGoal = INFINITY;
			nodes[y*GridWidth + x].fLocalGoal = INFINITY;
			nodes[y*GridWidth + x].parent = nullptr;
		}
	}

	auto distance = [](ThetaNode* a, ThetaNode* b) // For convenience
	{
		return sqrtf((a->x - b->x)*(a->x - b->x) + (a->y - b->y)*(a->y - b->y));
	};
	auto heuristic = [distance](ThetaNode* a, ThetaNode* b)	//
	{
		return distance(a, b);
	};

	// Setup starting conditions
	ThetaNode *nodeCurrent = NodeStart;

	NodeStart->fLocalGoal = 0.0f;
	NodeStart->fGlobalGoal = heuristic(NodeStart, NodeEnd);

	// Add start node to not tested list - this will ensure it gets tested
	// as the algorithm progresses, newly discovered nodes get added to the
	// list, and will themselves be tested later
	TArray<ThetaNode*> listNotTestedNodes;
	listNotTestedNodes.Add(NodeStart);
	//listNotTestedNodes.sort([](const sNode* lhs, const sNode* rhs) { return lhs->fGlobalGoal < rhs->fGlobalGoal; });
	// while (listNotTestedNodes.Num() > 0 && nodeCurrent != NodeEnd) <- this code will find the path faster but it may not be the shortest path
	while (listNotTestedNodes.Num() > 0 && nodeCurrent != NodeEnd)
	{
		// Sort Untested nodes by global goal, so lowest is first
		listNotTestedNodes.Sort([](const ThetaNode& lhs, const ThetaNode& rhs) {return lhs.fGlobalGoal < rhs.fGlobalGoal; });
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

		//	Update Vertex
		nodeCurrent = listNotTestedNodes[0];
		nodeCurrent->bVisited = true;	// We only explore a node once

		// Check each of this nodes neighbours...
		// IT MIGHT BE SMART TO JUST INITIALIZE EVERY NEIGHBOUR THAT WE ARE LOOKING AT INSTEAD OF THE WHOLE GRAPH!!!!!!!!
		// LOOK AT THETA* ALGORITHM ON WIKI
		for (auto nodeNeighbour : nodeCurrent->NeighbourNodes)
		{
			if (!nodeNeighbour->bVisited && nodeNeighbour->bObstacle == 0)
			{
				// Calculate the neighbours potential lowest parent distance
				float fPossiblyLowerGoal = nodeCurrent->fLocalGoal + distance(nodeCurrent, nodeNeighbour);
				// This if statement is very long but I cant make the two vectors before I check if nodeCurrent->parent is not null
				if (nodeCurrent->parent != nullptr &&
					IsClearPath(FVector(nodeCurrent->parent->x, nodeCurrent->parent->y, FloorHeight), FVector(nodeNeighbour->x, nodeNeighbour->y, FloorHeight)))
				{
					fPossiblyLowerGoal = nodeCurrent->parent->fLocalGoal + distance(nodeCurrent->parent, nodeNeighbour);
					if (fPossiblyLowerGoal < nodeNeighbour->fLocalGoal)
					{
						nodeNeighbour->parent = nodeCurrent->parent;
						nodeNeighbour->fLocalGoal = fPossiblyLowerGoal;
						nodeNeighbour->fGlobalGoal = nodeNeighbour->fLocalGoal + heuristic(nodeNeighbour, NodeEnd);
					}
				}
				else
				{
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
						nodeNeighbour->fGlobalGoal = nodeNeighbour->fLocalGoal + heuristic(nodeNeighbour, NodeEnd);
					}
				}
				listNotTestedNodes.Add(nodeNeighbour);
			}
		}
	}

	/*
	FColor DebugCol;
	for (int x = 0; x < GridWidth; x++)
	{
		for (int y = 0; y < GridHeight; y++)
		{
			if (nodes[y*GridWidth + x].bVisited == true)
			{
				DebugCol = FColor(0, 255, 255);
			}
			else if (&nodes[y*GridWidth + x] == NodeStart)
			{
				DebugCol = FColor(0, 255, 0);
			}
			else if (&nodes[y*GridWidth + x] == NodeEnd)
			{
				DebugCol = FColor(255, 0, 0);
			}
			else
			{
				DebugCol = FColor(0, 0, 255);
			}
			DrawDebugSphere(GetWorld(), FVector(x * NodeDist, y * NodeDist, 180.f), 30, 10, DebugCol, true); //////////// For drawing node representations


		}
	}
	*/

	// Draw Path by starting path the end, and following the parent node trail
	// I need to make it so that this piece of code only runs if the player can see the enemy
	if (NodeEnd != nullptr)
	{
		ThetaNode* p = NodeEnd;
		while (p->parent != nullptr)
		{
			EnemyPath.Add(p);
			//DrawDebugLine(GetWorld(), FVector(p->x * NodeDist, p->y * NodeDist, FloorHeight+1), FVector(p->parent->x * NodeDist, p->parent->y * NodeDist, FloorHeight+1), FColor(0, 0, 255), false, 0.5);
			p = p->parent;
		}
	}



	//	Only update InterpLocation if the enemy has a path to the goal node
	if (EnemyPath.Num() > 0)
	{
		UpdateInterpLocation();
	}


}

void UEnemyBrain::UpdateInterpLocation()
{
	// Only update InterpLocation if the enemy has a path to follow
	if (EnemyPath.Num() > 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Moving to next node..."));
		ThetaNode* InterpNode = EnemyPath.Pop();
		InterpLocation = FVector(InterpNode->x, InterpNode->y, EnemyLocation.Z);
	}
	else
	{
		//	No new nodes to move to...
	}
}

void UEnemyBrain::ArrivedInterpLoc()
{
	// Check if Enemy has made it to the node its moving to
	if (EnemyLocation.Equals(InterpLocation, 0.1f))
	{
		//Only run UpdateInterpLocation() if there is a path to follow
		if (EnemyPath.Num() > 0)
		{
			// Try to update interp location
			UpdateInterpLocation();
		}
	}
}

bool UEnemyBrain::IsClearPath(FVector Start, FVector End)
{
	//	These are for the point translation so we can get points StartR, StartL, EndR, and EndL
	//	(x2,y2) = point EnemyHalfWidth away from origin translated to be at same angle as enemy is facing
	//	(x1,y1) = rotate point (x2,y2) 270 degrees and then translate to Enemy location so that it lies on Enemy's right bounding box. 
	//	(x3,y3) = same as above but only rotate 90 degrees.
	//	(x4,y4) / (x5,y5) = apply same translations from End point so the lines cast parallel
	FVector Direction = (End - Start);
	Direction.Normalize();
	const float x2 = (cos(Direction.Rotation().Yaw * (PI / 180)) * EnemyHalfWidth);
	const float y2 = (sin(Direction.Rotation().Yaw * (PI / 180)) * EnemyHalfWidth);
	const float x1 = x2 * cos(270 * (PI / 180)) - y2 * sin(270 * (PI / 180)) + Start.X;
	const float y1 = y2 * cos(270 * (PI / 180)) + x2 * sin(270 * (PI / 180)) + Start.Y;
	const float x3 = x2 * cos(90 * (PI / 180)) - y2 * sin(90 * (PI / 180)) + Start.X;
	const float y3 = y2 * cos(90 * (PI / 180)) + x2 * sin(90 * (PI / 180)) + Start.Y;
	const float x4 = x2 * cos(90 * (PI / 180)) - y2 * sin(90 * (PI / 180)) + End.X;
	const float y4 = y2 * cos(90 * (PI / 180)) + x2 * sin(90 * (PI / 180)) + End.Y;
	const float x5 = x2 * cos(270 * (PI / 180)) - y2 * sin(270 * (PI / 180)) + End.X;
	const float y5 = y2 * cos(270 * (PI / 180)) + x2 * sin(270 * (PI / 180)) + End.Y;
	// This is the point that lies on the right side of the enemy's bounding box
	FVector StartR = FVector(x1, y1, FloorHeight);
	// This is the point that lies on the left side of the enemy's bounding box
	FVector StartL = FVector(x3, y3, FloorHeight);
	FVector EndR = FVector(x5, y5, FloorHeight);
	FVector EndL = FVector(x4, y4, FloorHeight);
	//DrawDebugLine(GetWorld(), StartL, EndL, FColor(255, 0, 0), false, 0.1f);
	//DrawDebugLine(GetWorld(), StartR, EndR, FColor(255, 0, 0), false, 0.1f);
	if (GetWorld()->LineTraceSingleByChannel(HitStruct, StartR, EndR, ECC_WorldDynamic, RayCollisionParams) == true)
	{
		return false;
	}
	else if (GetWorld()->LineTraceSingleByChannel(HitStruct, StartL, EndL, ECC_WorldDynamic, RayCollisionParams) == true)
	{
		return false;
	}
	return true;
}

bool UEnemyBrain::InPlayerLOS()
{
	return false;
}


