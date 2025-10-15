// BoomerangTarget.cpp

#include "BoomerangTarget.h"
#include "Components/StaticMeshComponent.h"
#include "BoomerangActor.h"
#include "Kismet/GameplayStatics.h"

// Constructor
ABoomerangTarget::ABoomerangTarget()
{
    PrimaryActorTick.bCanEverTick = false;

    TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
    RootComponent = TargetMesh;

    TargetMesh->SetCollisionObjectType(ECC_WorldDynamic);

    // Query only and overlap with boomerang
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TargetMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    TargetMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap); // allow boomerang overlap
    TargetMesh->SetGenerateOverlapEvents(true);

    TargetMesh->SetSimulatePhysics(false);

    // Bind begin overlap (not hit)
    TargetMesh->OnComponentBeginOverlap.AddDynamic(this, &ABoomerangTarget::OnOverlapBegin);
}


void ABoomerangTarget::BeginPlay()
{
    Super::BeginPlay();
}

void ABoomerangTarget::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Check if hit by boomerang
    if (OtherActor && OtherActor->IsA(ABoomerangActor::StaticClass()))
    {
        // (Optional) Play an effect or sound
        // UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, GetActorLocation());
        // UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());

        // Destroy the target immediately
        Destroy();
    }
}


UFUNCTION()
void ABoomerangTarget::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor->IsA(ABoomerangActor::StaticClass()))
    {
        UE_LOG(LogTemp, Log, TEXT("Target overlapped by boomerang: %s"), *GetName());

        Destroy();
    }
}

