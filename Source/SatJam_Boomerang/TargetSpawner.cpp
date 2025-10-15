// TargetSpawner.cpp


#include "TargetSpawner.h"
#include "BoomerangTarget.h"

// Sets default values
ATargetSpawner::ATargetSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATargetSpawner::BeginPlay()
{
	Super::BeginPlay();
	
    // Start a repeating timer that calls SpawnTarget() every few seconds
    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &ATargetSpawner::SpawnTarget,
        SpawnInterval,
        true // loop
    );
}

// Called every frame
void ATargetSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATargetSpawner::SpawnTarget()
{
    // Safety check: Make sure we have a valid enemy class set
    if (!TargetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetSpawner: TargetClass not set!"));
        return;
    }

    // Calculate random spawn position within radius
    FVector Origin = GetActorLocation();
    
	float Angle = FMath::FRandRange(0.0f, 2 * PI); // Random angle in radians

	float Distance = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius); // Random distance from the spawner

	// convert polar to cartesian coordinates
	float X = Distance * FMath::Cos(Angle);
	float Y = Distance * FMath::Sin(Angle);

	float Z = FMath::FRandRange(MinSpawnHeight, MaxSpawnHeight); // Random height

	FVector SpawnLocation = Origin + FVector(X, Y, Z);

    // Default rotation (no rotation needed)
    FRotator SpawnRotation = FRotator::ZeroRotator;

    // Spawn parameters (safe spawn)
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Actually spawn the enemy
    ABoomerangTarget* SpawnedTarget = GetWorld()->SpawnActor<ABoomerangTarget>(
        TargetClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (SpawnedTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target spawned at: %s"), *SpawnLocation.ToString());
    }
}


void ATargetSpawner::StopSpawning()
{
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
    UE_LOG(LogTemp, Warning, TEXT("%s: Spawning stopped."), *GetName());
}