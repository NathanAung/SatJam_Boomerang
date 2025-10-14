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

    // Enable collision
    BoomerangMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoomerangMesh->SetCollisionObjectType(ECC_PhysicsBody);
    BoomerangMesh->SetCollisionResponseToAllChannels(ECR_Block);
    BoomerangMesh->SetNotifyRigidBodyCollision(true); // allow OnHit to trigger
    BoomerangMesh->SetGenerateOverlapEvents(false);

    BoomerangMesh->SetSimulatePhysics(true);

    // Bind OnHit event
    BoomerangMesh->OnComponentHit.AddDynamic(this, &ABoomerangActor::OnHit);
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


void ABoomerangActor::InitializeWithPath(const TArray<FVector>& InPath, APlayerPawnBoomerang* Player)
{
    PathPoints = InPath;
    PlayerRef = Player;
    bFollowingPath = PathPoints.Num() >= 2;
    PathTime = 0.f;

    // while following path we control position, so disable physics
    BoomerangMesh->SetSimulatePhysics(false);
}

void ABoomerangActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bHasHitGround) return;

    if (bFollowingPath && PathPoints.Num() >= 2)
    {
        PathTime += DeltaTime;
        float Alpha = FMath::Clamp(PathTime / TotalFlightTime, 0.f, 1.f);

        int32 NumSegments = PathPoints.Num() - 1;
        float SegF = Alpha * NumSegments;
        int32 SegIndex = FMath::Clamp(FMath::FloorToInt(SegF), 0, NumSegments - 1);
        float LocalT = SegF - SegIndex;

        FVector StartPos = PathPoints[SegIndex];
        FVector EndPos = PathPoints[SegIndex + 1];
        FVector DesiredPos = FMath::Lerp(StartPos, EndPos, LocalT);

        // Move with sweep so collisions are detected
        FHitResult Hit;
        SetActorLocation(DesiredPos, true, &Hit);

        // Visual spin
        AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));

        // If we hit something while sweeping, switch to physics (react naturally) and schedule destruction
        if (Hit.bBlockingHit)
        {
            bHasHitGround = true;
            bFollowingPath = false;

            BoomerangMesh->SetSimulatePhysics(true);

            // optionally transfer small impulse so it doesn't stick exactly where hit
            // BoommerangMesh->AddImpulseAtLocation(-Hit.ImpactNormal * 100.0f, Hit.ImpactPoint);

            SetLifeSpan(3.f);
            return;
        }

        // reached end of path
        if (Alpha >= 1.f)
        {
            Destroy();
            return;
        }
    }
    else
    {
        // physics-driven behavior (if it fell to ground)
        AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));
    }
}




void ABoomerangActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UE_LOG(LogTemp, Warning, TEXT("Boomerang hit the ground!"));
    if (bHasHitGround) return; // prevent multiple triggers

    // Check if we hit something with the "WorldStatic" type (ground, walls, etc.)
    if (OtherActor && OtherActor != this && OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
    {
        UE_LOG(LogTemp, Warning, TEXT("Boomerang hit the ground!"));
        bHasHitGround = true;

        // Stop scripted flight
        SetActorTickEnabled(false);

        // Schedule destruction
        SetLifeSpan(3.0f);
    }
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
