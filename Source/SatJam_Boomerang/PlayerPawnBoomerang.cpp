#include "PlayerPawnBoomerang.h"
#include "BoomerangActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

APlayerPawnBoomerang::APlayerPawnBoomerang()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    CapsuleComponent->InitCapsuleSize(55.f, 96.f);
    RootComponent = CapsuleComponent;

    // Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    ThirdPersonOffset = FVector(150.f, 0.f, 100.f); // behind & above player
    Camera->SetRelativeLocation(ThirdPersonOffset);
    Camera->bUsePawnControlRotation = false; // manual rotation

    TrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySpline"));
    TrajectorySpline->SetupAttachment(RootComponent);
    TrajectorySpline->SetVisibility(false);
}

void APlayerPawnBoomerang::BeginPlay()
{
    Super::BeginPlay();

    // Lock mouse & hide cursor
    //if (APlayerController* PC = Cast<APlayerController>(GetController()))
    //{
    //    PC->bShowMouseCursor = true;
    //    PC->SetInputMode(FInputModeGameOnly());
    //}
}

void APlayerPawnBoomerang::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateTrajectoryPreview();

    // Apply rotation to camera
    Camera->SetWorldRotation(ControlRotation);
}

void APlayerPawnBoomerang::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("Turn", this, &APlayerPawnBoomerang::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawnBoomerang::LookUp);
    PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &APlayerPawnBoomerang::ThrowBoomerang);
}

void APlayerPawnBoomerang::Turn(float Value)
{
    if (Value != 0.f)
    {
        ControlRotation.Yaw += Value;
    }
}

void APlayerPawnBoomerang::LookUp(float Value)
{
    if (Value != 0.f)
    {
        ControlRotation.Pitch = FMath::Clamp(ControlRotation.Pitch + Value, -89.f, 89.f);
    }
}

void APlayerPawnBoomerang::ThrowBoomerang()
{
    if (!BoomerangClass || ActiveBoomerang) return;

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
        TrajectorySpline->SetVisibility(false);
    }
}

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

    // Start from player location
    FVector Start = GetActorLocation();
    FVector Forward = ControlRotation.Vector().GetSafeNormal(); // use camera rotation
    FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

    float UseDistance = Distance;
    float UseCurveRadius = CurveRadius;

    if (BoomerangClass)
    {
        if (ABoomerangActor* CDO = Cast<ABoomerangActor>(BoomerangClass->GetDefaultObject()))
        {
            UseDistance = CDO->Distance;
            UseCurveRadius = CDO->CurveRadius;
        }
    }

    // Sample trajectory
    for (int32 i = 0; i <= NumSplinePoints; ++i)
    {
        float T = (NumSplinePoints > 0) ? (static_cast<float>(i) / NumSplinePoints) : 0.f;

        float sinPI_T = FMath::Sin(T * PI);
        float sideSin = FMath::Sin(T * 2.f * PI);

        FVector Outward = Start + Forward * (sinPI_T * UseDistance);
        FVector SideOffset = Right * (sideSin * UseCurveRadius);

        FVector Point = Outward + SideOffset;
        TrajectorySpline->AddSplinePoint(Point, ESplineCoordinateSpace::World);
    }

    TrajectorySpline->SetVisibility(true);

    // Debug lines
    for (int32 i = 1; i <= NumSplinePoints; ++i)
    {
        FVector P1 = TrajectorySpline->GetLocationAtSplinePoint(i - 1, ESplineCoordinateSpace::World);
        FVector P2 = TrajectorySpline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        DrawDebugLine(GetWorld(), P1, P2, FColor::Green, false, -1.f, 0, 2.f);
    }
}

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

void APlayerPawnBoomerang::NotifyOwnerDestroyed()
{
    ActiveBoomerang = nullptr;
    TrajectorySpline->SetVisibility(true); // preview visible again
}
