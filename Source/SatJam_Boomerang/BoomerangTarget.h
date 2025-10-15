// BoomerangTarget.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoomerangTarget.generated.h"

UCLASS()
class SATJAM_BOOMERANG_API ABoomerangTarget : public AActor
{
    GENERATED_BODY()

public:
    ABoomerangTarget();

protected:
    virtual void BeginPlay() override;

private:
    // Static mesh for visual representation
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* TargetMesh;

    UPROPERTY(EditAnywhere, Category = "Target")
    float lifeTime = 5.0f;

    // Handles hit events
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    // Called on overlap begin (used to detect targets)
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};
