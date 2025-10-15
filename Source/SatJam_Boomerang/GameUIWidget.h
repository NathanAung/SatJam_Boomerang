// GameUIWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameUIWidget.generated.h"

UCLASS()
class SATJAM_BOOMERANG_API UGameUIWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // TextBlocks bound from Blueprint
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TimeText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ScoreText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* RetryText;

    void UpdateTime(int32 SecondsLeft);
    void UpdateScore(int32 NewScore);
    void ShowGameOverMessage();
};
