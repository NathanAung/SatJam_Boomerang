#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerPawnBoomerang.h"
#include "BoomerangActor.generated.h"

UCLASS()
class SATJAM_BOOMERANG_API ABoomerangActor : public AActor
{
    GENERATED_BODY()

public:
    ABoomerangActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

private:
    // Mesh for visual representation
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* BoomerangMesh;

    // Movement speed of the boomerang
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float ForwardSpeed = 1200.f;

    // How strong the boomerang curves back
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float CurveStrength = 2.0f;

    // Time before the boomerang starts returning
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float ReturnDelay = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Boomerang")
	float TotalFlightTime = 2.5f;   // total time before returning
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float Distance = 1000.f; // how far it travels outward
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float CurveRadius = 300.f; // how wide the curve is

    // Internal timer for switching direction
    float ElapsedTime = 0.0f;

    // Direction it was thrown
    FVector InitialForwardDirection;

    // Reference to the player (to return to)
    APlayerPawnBoomerang* PlayerRef = nullptr;

    virtual void Destroyed() override;


public:
    void InitializeBoomerang(const FVector& Direction, APlayerPawnBoomerang* Player);

};
