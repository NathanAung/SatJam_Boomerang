// GameUIWidget.cpp

#include "GameUIWidget.h"
#include "Components/TextBlock.h"


void UGameUIWidget::UpdateTime(int32 SecondsLeft)
{
    if (TimeText)
    {
        TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %d"), SecondsLeft)));
    }
}


void UGameUIWidget::UpdateScore(int32 NewScore)
{
    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), NewScore)));
    }
}


void UGameUIWidget::ShowGameOverMessage()
{
    if (StatusText)
    {
        StatusText->SetVisibility(ESlateVisibility::Visible);
        RetryText->SetVisibility(ESlateVisibility::Visible);
    }
}
