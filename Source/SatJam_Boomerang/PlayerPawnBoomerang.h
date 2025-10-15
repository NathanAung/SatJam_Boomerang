// PlayerPawnBoomerang.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "PlayerPawnBoomerang.generated.h"

class UCameraComponent;
class ABoomerangActor;

UCLASS()
class SATJAM_BOOMERANG_API APlayerPawnBoomerang : public APawn
{
    GENERATED_BODY()

public:
    APlayerPawnBoomerang();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Called by the boomerang when destroyed so the pawn can update state
    void NotifyOwnerDestroyed();

private:
    /** Components */
    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    USplineComponent* TrajectorySpline;

    // Active boomerang spawned by player
    ABoomerangActor* ActiveBoomerang = nullptr;

    // Control rotation stored manually for camera orientation
    FRotator ControlRotation;

    // Camera offset for third-person view
    FVector ThirdPersonOffset;

    // Trajectory spline parameters
    UPROPERTY(EditAnywhere, Category = "Boomerang Trajectory")
    int32 NumSplinePoints = 20;

    UPROPERTY(EditAnywhere, Category = "Boomerang Trajectory")
    float Distance = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Boomerang Trajectory")
    float CurveRadius = 300.f;

    // Boomerang class to spawn
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    TSubclassOf<ABoomerangActor> BoomerangClass;

    // Input callbacks
    void LookUp(float Value);
    void Turn(float Value);
    void ThrowBoomerang();

    // Update spline preview based on camera rotation
    void UpdateTrajectoryPreview();

    // Sample spline points into an array
    TArray<FVector> SampleTrajectoryPoints() const;
};
