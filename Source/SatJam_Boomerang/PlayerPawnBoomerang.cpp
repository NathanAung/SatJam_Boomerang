#include "PlayerPawnBoomerang.h"
#include "BoomerangActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

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
    Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // roughly head height
    Camera->bUsePawnControlRotation = false; // we’ll handle rotation manually

    TrajectorySpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrajectorySpline"));
    TrajectorySpline->SetupAttachment(RootComponent);
    TrajectorySpline->SetVisibility(false);

}

void APlayerPawnBoomerang::BeginPlay()
{
    Super::BeginPlay();

    // Lock mouse to viewport and show/hide cursor
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void APlayerPawnBoomerang::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	UpdateTrajectoryPreview();

    // Apply the control rotation to the camera
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

    // Ensure preview is current and sample it
    UpdateTrajectoryPreview();
    TArray<FVector> PathPoints = SampleTrajectoryPoints();
    if (PathPoints.Num() < 2) return;

    // Spawn at first sampled point so positions line up
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

    // Hide preview while a boomerang is active
    if (ActiveBoomerang)
    {
        TrajectorySpline->SetVisibility(false);
        TrajectorySpline->ClearSplinePoints();
        return;
    }

    TrajectorySpline->ClearSplinePoints();

    // Start from camera location so preview matches spawn origin
    FVector Start = Camera->GetComponentLocation();
    FVector Forward = Camera->GetForwardVector().GetSafeNormal();
    FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

    // Use the boomerang settings from the class defaults if available so preview matches runtime
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

    // Sample parametric path T in [0..1] and add points to spline
    for (int32 i = 0; i <= NumSplinePoints; ++i)
    {
        float T = (NumSplinePoints > 0) ? (static_cast<float>(i) / NumSplinePoints) : 0.f;

        // EXACT same math as boomerang path (horizontal only)
        float sinPI_T = FMath::Sin(T * PI);             // outward profile
        float sideSin = FMath::Sin(T * 2.0f * PI);     // side wobble

        FVector Outward = Start + Forward * (sinPI_T * UseDistance);
        FVector SideOffset = Right * (sideSin * UseCurveRadius);

        FVector Point = Outward + SideOffset; // NO vertical component

        TrajectorySpline->AddSplinePoint(Point, ESplineCoordinateSpace::World);
    }

    TrajectorySpline->SetVisibility(true);

    // DEBUG: draw lines between points
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
	TrajectorySpline->SetVisibility(false);
}
