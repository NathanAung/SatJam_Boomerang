// GameManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameUIWidget.h"
#include "GameManager.generated.h"

UCLASS()
class SATJAM_BOOMERANG_API AGameManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGameManager();

	int32 Score = 0;

	void AddScore(int32 Points);

    // Add UI reference
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UGameUIWidget> GameUIClass;

    UPROPERTY()
    UGameUIWidget* GameUI;

    // Timer update helper
    void UpdateUI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
    // How long the game lasts in seconds (1 minute = 60 seconds)
    UPROPERTY(EditAnywhere, Category = "Game Rules")
    float GameDuration = 60.0f;

    // Timer handle for the main game timer
    FTimerHandle GameTimerHandle;

    // Called when the timer ends
    void OnGameEnd();

	// Helper function to stop target spawning
    void StopSpawner();

    // Helper function to destroy all existing targets
    void DestroyAllTargets();

    UFUNCTION()
    void RestartGame();

    UFUNCTION()
    void QuitGame();

    bool gameEnded = false;
};
