// BoomerangActor.cpp

#include "BoomerangActor.h"
#include "GameManager.h"
#include "Components/StaticMeshComponent.h"
#include "BoomerangTarget.h"
#include "Kismet/GameplayStatics.h"

ABoomerangActor::ABoomerangActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // Visual mesh
    BoomerangMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoomerangMesh"));
    RootComponent = BoomerangMesh;

    // Collision setup
    // Block world (so ground/walls stop the boomerang when sweeping)
    BoomerangMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoomerangMesh->SetCollisionObjectType(ECC_GameTraceChannel1);   // custom channel for boomerang
    BoomerangMesh->SetCollisionResponseToAllChannels(ECR_Block);
    BoomerangMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // targets use this channel
    BoomerangMesh->SetGenerateOverlapEvents(true);

	// Don't simulate physics initially
    BoomerangMesh->SetSimulatePhysics(false);

    // Bind overlap for targets and hit for blocking world collisions (when physics enabled)
    BoomerangMesh->OnComponentBeginOverlap.AddDynamic(this, &ABoomerangActor::OnBeginOverlap);
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
        SetActorLocation(DesiredPos, true, &Hit); // sweep enabled

        // Visual spin
        AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));

        if (Hit.bBlockingHit)
        {
            UPrimitiveComponent* HitComp = Hit.GetComponent();
            AActor* HitActor = Hit.GetActor();
            ECollisionChannel ObjType = HitComp ? HitComp->GetCollisionObjectType() : ECC_Visibility;

            UE_LOG(LogTemp, Verbose, TEXT("Sweep hit actor=%s objType=%d"), HitActor ? *HitActor->GetName() : TEXT("None"), (int32)ObjType);

            // If this is a world static (ground/wall) collision, stop and enable physics
            if (ObjType == ECC_WorldStatic)
            {
                bHasHitGround = true;
                bFollowingPath = false;
                BoomerangMesh->SetSimulatePhysics(true); // now physics reacts
                SetLifeSpan(3.f);
                return;
            }
            // If it's the target channel (GameTraceChannel1) it shouldn't be blocking.
            // If you ever see the target show up here, it means its object type/response isn't set correctly.
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
        // Physics fallback — just spinning visually
        AddActorLocalRotation(FRotator(0.f, 720.f * DeltaTime, 0.f));
    }
}


// Collision callback
void ABoomerangActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bHasHitGround) return;

    if (OtherActor && OtherActor->IsA(ABoomerangTarget::StaticClass()))
    {
        UE_LOG(LogTemp, Log, TEXT("Boomerang hit target, passing through"));

        // Disable collision between the boomerang and the target instantly
        if (OtherComp)
        {
            UE_LOG(LogTemp, Log, TEXT("Other comp"));
            HitComp->IgnoreActorWhenMoving(OtherActor, true);
            OtherComp->IgnoreActorWhenMoving(this, true);
        }

        return; // don’t apply any hit logic
    }

    // Handle ground or walls (stop)
    if (OtherActor && OtherComp && OtherComp->GetCollisionObjectType() == ECC_WorldStatic)
    {
        UE_LOG(LogTemp, Log, TEXT("Boomerang hit ground"));
        bHasHitGround = true;
        SetActorTickEnabled(false);
        SetLifeSpan(3.0f);
    }
}


void ABoomerangActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;

    // If we overlapped a boomerang target, destroy the target and keep flying.
    if (ABoomerangTarget* Target = Cast<ABoomerangTarget>(OtherActor))
    {
        UE_LOG(LogTemp, Log, TEXT("Boomerang overlapped target: %s"), *Target->GetName());


        // Option A: let the target destroy itself (preferred)
        Target->Destroy();

        // Option B: if target has special logic, call it here instead of destroying directly.
        // Target->OnHitByBoomerang();

        // IMPORTANT: do not enable physics or stop following path here.
        // We want the boomerang to pass through cleanly.

        // Award points through GameManager
        AGameManager* GameManager = Cast<AGameManager>(
            UGameplayStatics::GetActorOfClass(GetWorld(), AGameManager::StaticClass())
        );

        if (GameManager)
        {
            GameManager->AddScore(100);
        }
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




