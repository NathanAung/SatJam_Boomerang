// TargetSpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoomerangTarget.h"
#include "TargetSpawner.generated.h"

UCLASS()
class SATJAM_BOOMERANG_API ATargetSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATargetSpawner();

	void StopSpawning();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
    // The enemy type to spawn
    // You can set this in the Unreal Editor (Blueprint defaults panel)
    UPROPERTY(EditAnywhere, Category = "Spawner")
    TSubclassOf<ABoomerangTarget> TargetClass;

    // How often to spawn enemies (in seconds)
    UPROPERTY(EditAnywhere, Category = "Spawner")
    float SpawnInterval = 5.0f;

    // minumum distance for how far away from the spawner targets can appear (for random spawn)
    UPROPERTY(EditAnywhere, Category = "Spawner")
    float MinSpawnRadius = 200.0f;
    // maximum distance for how far away from the spawner targets can appear (for random spawn)
    UPROPERTY(EditAnywhere, Category = "Spawner")
    float MaxSpawnRadius = 900.0f;
	// minumum height for how far away from the spawner targets can appear (for random spawn)
    UPROPERTY(EditAnywhere, Category = "Spawner")
    float MinSpawnHeight = 40.0f;
	// maximum height for how far away from the spawner targets can appear (for random spawn)
    UPROPERTY(EditAnywhere, Category = "Spawner")
    float MaxSpawnHeight = 600.0f;

    // Timer handle to repeatedly call the spawn function
    FTimerHandle SpawnTimerHandle;

    // Function that actually spawns the enemy
    void SpawnTarget();
};
