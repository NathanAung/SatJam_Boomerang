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

    FVector SpawnLocation = Camera->GetComponentLocation() + Camera->GetForwardVector() * 100.f;
    FRotator SpawnRotation = Camera->GetComponentRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;

    ABoomerangActor* Boomerang = GetWorld()->SpawnActor<ABoomerangActor>(BoomerangClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (Boomerang)
    {
        Boomerang->InitializeBoomerang(Camera->GetForwardVector(), this);
        ActiveBoomerang = Boomerang;
    }
}


void APlayerPawnBoomerang::NotifyOwnerDestroyed()
{
    ActiveBoomerang = nullptr;
}
