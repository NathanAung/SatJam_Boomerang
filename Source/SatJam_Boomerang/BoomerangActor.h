// BoomerangActor.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerPawnBoomerang.h"
#include "BoomerangActor.generated.h"

UCLASS()
class SATJAM_BOOMERANG_API ABoomerangActor : public AActor
{
    GENERATED_BODY()

public:
    ABoomerangActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Destroyed() override;

private:
    /** Visual mesh */
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* BoomerangMesh;

    /** Movement parameters */
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float ForwardSpeed = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float TotalFlightTime = 2.5f;

    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float SweepRadius = 12.f;

    float ElapsedTime = 0.f;
    bool bHasHitGround = false;

    /** Path-following */
    TArray<FVector> PathPoints;
    bool bFollowingPath = false;
    float PathTime = 0.f;

    /** Direction and player reference */
    FVector InitialForwardDirection;
    APlayerPawnBoomerang* PlayerRef = nullptr;

public:
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float Distance = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Boomerang")
    float CurveRadius = 300.f;

    /** Initialize with direction for physics-driven flight */
    void InitializeBoomerang(const FVector& Direction, APlayerPawnBoomerang* Player);

    /** Initialize using precomputed path points */
    void InitializeWithPath(const TArray<FVector>& InPath, APlayerPawnBoomerang* Player);

    /** Called on collision */
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
