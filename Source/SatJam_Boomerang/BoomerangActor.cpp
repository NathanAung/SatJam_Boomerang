// BoomerangActor.cpp

#include "BoomerangActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ABoomerangActor::ABoomerangActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Visual mesh
    BoomerangMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoomerangMesh"));
    RootComponent = BoomerangMesh;

    // Enable collision for OnHit
    BoomerangMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BoomerangMesh->SetCollisionObjectType(ECC_PhysicsBody);
    BoomerangMesh->SetCollisionResponseToAllChannels(ECR_Block);
    BoomerangMesh->SetNotifyRigidBodyCollision(true);
    BoomerangMesh->SetSimulatePhysics(true);

    BoomerangMesh->OnComponentHit.AddDynamic(this, &ABoomerangActor::OnHit);
}

void ABoomerangActor::BeginPlay()
{
    Super::BeginPlay();
}

// --- Initialize for physics-driven flight ---
void ABoomerangActor::InitializeBoomerang(const FVector& Direction, APlayerPawnBoomerang* Player)
{
    InitialForwardDirection = Direction;
    PlayerRef = Player;
}

// --- Initialize using precomputed spline path ---
void ABoomerangActor::InitializeWithPath(const TArray<FVector>& InPath, APlayerPawnBoomerang* Player)
{
    PathPoints = InPath;
    PlayerRef = Player;
    bFollowingPath = PathPoints.Num() >= 2;
    PathTime = 0.f;

    // Disable physics during scripted path
    BoomerangMesh->SetSimulatePhysics(false);
}

void ABoomerangActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bHasHitGround) return;

    // Follow the precomputed path
    if (bFollowingPath && PathPoints.Num() >= 2)
    {
        PathTime += DeltaTime;
        float Alpha = FMath::Clamp(PathTime / TotalFlightTime, 0.f, 1.f);

        int32 NumSegments = PathPoints.Num() - 1;
        float SegF = Alpha * NumSegments;
        int32 SegIndex = FMath::Clamp(FMath::FloorToInt(SegF), 0, NumSegments - 1);
        float LocalT = SegF - SegIndex;

        FVector DesiredPos = FMath::Lerp(PathPoints[SegIndex], PathPoints[SegIndex + 1], LocalT);

        FHitResult Hit;
        SetActorLocation(DesiredPos, true, &Hit);

        // Visual spin
        AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));

        // Hit something while moving along path
        if (Hit.bBlockingHit)
        {
            bHasHitGround = true;
            bFollowingPath = false;
            BoomerangMesh->SetSimulatePhysics(true);
            SetLifeSpan(3.f);
            return;
        }

        // End of path reached
        if (Alpha >= 1.f)
        {
            Destroy();
            return;
        }
    }
    else
    {
        // Physics-driven fallback
        AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));
    }
}

// Collision callback
void ABoomerangActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bHasHitGround) return;

    if (OtherActor && OtherActor != this && OtherComp &&
        OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
    {
        bHasHitGround = true;
        SetActorTickEnabled(false);
        SetLifeSpan(3.f);
    }
}

void ABoomerangActor::Destroyed()
{
    Super::Destroyed();

    if (PlayerRef)
    {
        PlayerRef->NotifyOwnerDestroyed();
    }
}
