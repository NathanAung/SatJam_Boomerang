// PlayerPawnBoomerang.cpp

#include "PlayerPawnBoomerang.h"
#include "BoomerangActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"


APlayerPawnBoomerang::APlayerPawnBoomerang()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root capsule for collision
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    CapsuleComponent->InitCapsuleSize(55.f, 96.f);
    RootComponent = CapsuleComponent;

    // Camera (third-person offset)
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    ThirdPersonOffset = FVector(-100.f, 0.f, 100.f); // behind & above player
    Camera->SetRelativeLocation(ThirdPersonOffset);
    Camera->bUsePawnControlRotation = false;

    // Trajectory spline (visualizes boomerang path)
    TrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySpline"));
    TrajectorySpline->SetupAttachment(RootComponent);
    TrajectorySpline->SetVisibility(false);
}


void APlayerPawnBoomerang::BeginPlay()
{
    Super::BeginPlay();
}


void APlayerPawnBoomerang::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update trajectory preview every tick
    UpdateTrajectoryPreview();

    // Rotate the offset by the yaw/pitch of the control rotation
    FVector RotatedOffset = ControlRotation.RotateVector(ThirdPersonOffset);

    // Apply camera location and rotation
    Camera->SetWorldLocation(GetActorLocation() + RotatedOffset);
    Camera->SetWorldRotation(ControlRotation);

}


void APlayerPawnBoomerang::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Mouse/controller input
    PlayerInputComponent->BindAxis("Turn", this, &APlayerPawnBoomerang::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawnBoomerang::LookUp);

    // Throw boomerang
    PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &APlayerPawnBoomerang::ThrowBoomerang);
}


// Input handlers
void APlayerPawnBoomerang::Turn(float Value)
{
    if (Value != 0.f)
        ControlRotation.Yaw += Value;
}

void APlayerPawnBoomerang::LookUp(float Value)
{
    if (Value != 0.f)
        ControlRotation.Pitch = FMath::Clamp(ControlRotation.Pitch + Value, -89.f, 89.f);
}


void APlayerPawnBoomerang::ThrowBoomerang()
{
    if (!BoomerangClass || ActiveBoomerang)
        return;

    // Ensure trajectory preview is up to date
    UpdateTrajectoryPreview();

    TArray<FVector> PathPoints = SampleTrajectoryPoints();
    if (PathPoints.Num() < 2) return;

    FVector SpawnLocation = PathPoints[0];
    FRotator SpawnRotation = Camera->GetComponentRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    ABoomerangActor* Boomerang = GetWorld()->SpawnActor<ABoomerangActor>(BoomerangClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (Boomerang)
    {
        Boomerang->InitializeWithPath(PathPoints, this);
        ActiveBoomerang = Boomerang;

        // Hide trajectory while boomerang is active
        TrajectorySpline->SetVisibility(false);
    }
}


// Trajectory spline preview
void APlayerPawnBoomerang::UpdateTrajectoryPreview()
{
    if (!TrajectorySpline) return;

    // Hide preview if boomerang exists
    if (ActiveBoomerang)
    {
        TrajectorySpline->SetVisibility(false);
        TrajectorySpline->ClearSplinePoints();
        return;
    }

    TrajectorySpline->ClearSplinePoints();

    // Use player's location as spline start
    FVector Start = GetActorLocation();
    FVector Forward = ControlRotation.Vector().GetSafeNormal();
    FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

    float UseDistance = Distance;
    float UseCurveRadius = CurveRadius;

    // Use class defaults if available
    if (BoomerangClass)
    {
        if (ABoomerangActor* CDO = Cast<ABoomerangActor>(BoomerangClass->GetDefaultObject()))
        {
            UseDistance = CDO->Distance;
            UseCurveRadius = CDO->CurveRadius;
        }
    }

    // Sample trajectory points along the path
    for (int32 i = 0; i <= NumSplinePoints; ++i)
    {
		// divide trajectory to evenly spaced points
		// T ranges from 0 to 1
        float T = NumSplinePoints > 0 ? static_cast<float>(i) / NumSplinePoints : 0.f;

		float sinPI_T = FMath::Sin(T * PI); // forward motion: goes forward halfway through then comes back (0 > 1 > 0)
		float sideSin = FMath::Sin(T * 2.f * PI);   // sideways swing: does a full right-left oscillation (0 > 1 > 0 > -1 > 0)

		FVector Outward = Start + Forward * (sinPI_T * UseDistance);    // scale forward motion with distance
		FVector SideOffset = Right * (sideSin * UseCurveRadius);    // scale sideways motion with curve radius

        TrajectorySpline->AddSplinePoint(Outward + SideOffset, ESplineCoordinateSpace::World);
    }

    TrajectorySpline->SetVisibility(true);

    // Draw debug lines for visual clarity
    for (int32 i = 1; i <= NumSplinePoints; ++i)
    {
        FVector P1 = TrajectorySpline->GetLocationAtSplinePoint(i - 1, ESplineCoordinateSpace::World);
        FVector P2 = TrajectorySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        DrawDebugLine(GetWorld(), P1, P2, FColor::Green, false, -1.f, 0, 2.f);
    }
}


// Sample spline points for boomerang path
TArray<FVector> APlayerPawnBoomerang::SampleTrajectoryPoints() const
{
    TArray<FVector> Points;
    if (!TrajectorySpline) return Points;

    const int32 Count = TrajectorySpline->GetNumberOfSplinePoints();
    for (int32 i = 0; i < Count; ++i)
    {
        Points.Add(TrajectorySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
    }
    return Points;
}


// Called by boomerang when destroyed
void APlayerPawnBoomerang::NotifyOwnerDestroyed()
{
    ActiveBoomerang = nullptr;
    TrajectorySpline->SetVisibility(true); // show preview again
}
