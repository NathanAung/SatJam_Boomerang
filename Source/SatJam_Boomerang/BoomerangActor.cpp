#include "BoomerangActor.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"


ABoomerangActor::ABoomerangActor()
{
    PrimaryActorTick.bCanEverTick = true;

	// Create and set up the mesh component
    BoomerangMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoomerangMesh"));
    RootComponent = BoomerangMesh;

    BoomerangMesh->SetSimulatePhysics(false);
    BoomerangMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
}


void ABoomerangActor::BeginPlay()
{
    Super::BeginPlay();
}


// Initialize the boomerang's direction and player reference
void ABoomerangActor::InitializeBoomerang(const FVector& Direction, APlayerPawnBoomerang* Player)
{
    InitialForwardDirection = Direction;
    PlayerRef = Player;
}


void ABoomerangActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!PlayerRef) return;

    ElapsedTime += DeltaTime;

    // Time-based progress (normalized)
    float T = ElapsedTime / TotalFlightTime;

    // Clamp to 0–1
    if (T >= 1.0f)
    {
        Destroy();
        return;
    }

    // Define start and end points
    FVector Start = PlayerRef->GetActorLocation();
    FVector Forward = InitialForwardDirection;
    FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

    // Curve outward then back
    FVector Outward = Start + Forward * (FMath::Sin(T * PI) * Distance);
    FVector SideOffset = Right * FMath::Sin(T * 2 * PI) * CurveRadius;

    // Combine
    FVector TargetPos = Outward + SideOffset + FVector(0.f, 0.f, 50.f * FMath::Sin(T * PI)); // small vertical arc

    SetActorLocation(TargetPos);

    // Spin visually
    AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));
}


void ABoomerangActor::Destroyed()
{
    Super::Destroyed();

    if (PlayerRef)
    {
        // Let the player know the boomerang no longer exists
        PlayerRef->NotifyOwnerDestroyed();
    }
}
