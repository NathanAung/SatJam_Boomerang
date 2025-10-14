#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "PlayerPawnBoomerang.generated.h"

class UCameraComponent;
class ABoomerangActor;

UCLASS()
class SATJAM_BOOMERANG_API APlayerPawnBoomerang : public APawn
{
    GENERATED_BODY()

public:
    APlayerPawnBoomerang();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void NotifyOwnerDestroyed();


private:
    // Components
    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* CapsuleComponent;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    // Input
    void LookUp(float Value);
    void Turn(float Value);
    void ThrowBoomerang();

    // Boomerang class to spawn
    UPROPERTY(EditAnywhere, Category = "Boomerang")
    TSubclassOf<ABoomerangActor> BoomerangClass;

    // Pointer to active boomerang
    ABoomerangActor* ActiveBoomerang = nullptr;

    // Store control rotation manually
    FRotator ControlRotation;
};
