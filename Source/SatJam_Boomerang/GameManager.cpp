// Fill out your copyright notice in the Description page of Project Settings.


#include "GameManager.h"
#include "BoomerangTarget.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AGameManager::AGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGameManager::BeginPlay()
{
	Super::BeginPlay();
	
    // Force Windowed Mode
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("r.SetRes 1280x720w"));
    }


    Score = 0;

    // Start the main game timer
    GetWorldTimerManager().SetTimer(
        GameTimerHandle,
        this,
        &AGameManager::OnGameEnd,
        GameDuration,
        false // Only once
    );

    UE_LOG(LogTemp, Warning, TEXT("Game started. Timer set for %.1f seconds."), GameDuration);
}


// Called every frame
void AGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (gameEnded)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (!PC) return;

        PC->bShowMouseCursor = true;

        if (PC->WasInputKeyJustPressed(EKeys::SpaceBar))
        {
            RestartGame();
        }
        else if (PC->WasInputKeyJustPressed(EKeys::Escape))
        {
            QuitGame();
        }
    }
}


void AGameManager::AddScore(int32 Points)
{
    Score += Points;
    UE_LOG(LogTemp, Warning, TEXT("Score: %d"), Score);
}


void AGameManager::StopSpawner()
{
    
}


void AGameManager::DestroyAllTargets()
{
    TArray<AActor*> FoundTargets;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoomerangTarget::StaticClass(), FoundTargets);

    for (AActor* Actor : FoundTargets)
    {
        if (Actor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Destroying target: %s"), *Actor->GetName());
            Actor->Destroy();
        }
    }
}


void AGameManager::OnGameEnd()
{
    if (gameEnded) return; // Prevent multiple calls

    UE_LOG(LogTemp, Warning, TEXT("Time's up! Stopping spawners and clearing enemies."));

    StopSpawner();
    DestroyAllTargets();

    /*if (GameUI)
    {
        GameUI->ShowStatusMessage(TEXT("Game Clear"));
    }*/

    gameEnded = true;
}

void AGameManager::RestartGame()
{
    UE_LOG(LogTemp, Warning, TEXT("Restarting game..."));
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName())); // reload current level
}


void AGameManager::QuitGame()
{
    UE_LOG(LogTemp, Warning, TEXT("Quitting game..."));
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, true);
}