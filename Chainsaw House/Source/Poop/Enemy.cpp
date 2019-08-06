// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
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

// Sets default values
AEnemy::AEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(EnemyMesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EnemyMeshPath(TEXT("/Game/Geometry/Meshes/1M_Cube.1M_Cube"));
	if (EnemyMeshPath.Succeeded())
	{
		EnemyMesh->SetStaticMesh(EnemyMeshPath.Object);
		EnemyMesh->SetWorldScale3D(FVector(0.6f, 0.6f, 2.0f));
	}
	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	
}

//	Raycasts for a node to determine if it has a clear path from Start to End
//	Start node casts 2 parallel rays from its right and left side to End location.

// Raycast for enemy to determine if player can see it
// If Raycast hits nothing then enemy is in player LOS


// Function for updating the next target location for the VInterpTo function

// Is player inside FOV triangle?
bool AEnemy::IsInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{
	auto area = [](int x1, int y1, int x2, int y2, int x3, int y3) 
	{
		return abs((x1*(y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2);
	};
	/* Calculate area of triangle ABC */
	float A = area(x1, y1, x2, y2, x3, y3);

	/* Calculate area of triangle PBC */
	float A1 = area(x, y, x2, y2, x3, y3);

	/* Calculate area of triangle PAC */
	float A2 = area(x1, y1, x, y, x3, y3);

	/* Calculate area of triangle PAB */
	float A3 = area(x1, y1, x2, y2, x, y);

	/* Check if sum of A1, A2 and A3 is same as A */
	return A >= (A1 + A2 + A3) - 0.5f && A <= (A1 + A2 + A3) + 0.5f;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), InterpLocation.X, InterpLocation.Y, InterpLocation.Z);
}

