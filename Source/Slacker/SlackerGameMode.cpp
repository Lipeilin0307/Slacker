#include "SlackerGameMode.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

ASlackerGameMode::ASlackerGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASlackerGameMode::BeginPlay()
{
    Super::BeginPlay();
    GameStartTime = GetWorld()->GetTimeSeconds();

    if (HUDWidgetClass)
    {
        HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
            UpdateHUD();
        }
    }
}

void ASlackerGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateTimerDisplay();
    if (bPenaltyActive)
    {
        UpdatePenaltyDisplay();
    }
}

void ASlackerGameMode::UpdateHUD()
{
    if (!HUDWidget) return;

    UTextBlock* TextBlock = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Score")));
    if (TextBlock)
    {
        FString NewText = FString::Printf(TEXT("Slacking: %d / %d"), CompletedSlacks, TargetSlacks);
        TextBlock->SetText(FText::FromString(NewText));
    }
}

void ASlackerGameMode::UpdateTimerDisplay()
{
    if (!HUDWidget) return;

    UTextBlock* TimerText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Timer")));
    if (!TimerText) return;

    float CurrentTime = bGameEnded ? GameEndTime : GetWorld()->GetTimeSeconds();
    float Elapsed = CurrentTime - GameStartTime;

    int32 Minutes = FMath::FloorToInt(Elapsed / 60.0f);
    int32 Seconds = FMath::FloorToInt(Elapsed) % 60;
    int32 Millis = FMath::FloorToInt((Elapsed - FMath::FloorToInt(Elapsed)) * 100.0f);

    TimerText->SetText(FText::FromString(FString::Printf(TEXT("TIME: %02d:%02d.%02d"), Minutes, Seconds, Millis)));
}

void ASlackerGameMode::UpdatePenaltyDisplay()
{
    if (!HUDWidget) return;

    float Elapsed = GetWorld()->GetTimeSeconds() - PenaltyStartTime;
    float Remaining = FMath::Max(0.0f, 3.0f - Elapsed);
    float Percent = Remaining / 3.0f;

    if (UProgressBar* PenaltyBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(FName("ProgressBar_Penalty"))))
    {
        PenaltyBar->SetVisibility(ESlateVisibility::Visible);
        PenaltyBar->SetPercent(Percent);
    }

    if (UTextBlock* PenaltyText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Penalty"))))
    {
        PenaltyText->SetVisibility(ESlateVisibility::Visible);
        PenaltyText->SetText(FText::FromString(FString::Printf(TEXT("PENALTY: %.1fs"), Remaining)));
    }

    if (Remaining <= 0.0f)
    {
        StopPenaltyTimer();
    }
}

void ASlackerGameMode::StartPenaltyTimer()
{
    PenaltyStartTime = GetWorld()->GetTimeSeconds();
    bPenaltyActive = true;
}

void ASlackerGameMode::StopPenaltyTimer()
{
    bPenaltyActive = false;
    if (!HUDWidget) return;

    if (UProgressBar* PenaltyBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(FName("ProgressBar_Penalty"))))
    {
        PenaltyBar->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (UTextBlock* PenaltyText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Penalty"))))
    {
        PenaltyText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ASlackerGameMode::OnSlackCompleted()
{
    if (bGameWon) return;

    CompletedSlacks++;
    UpdateHUD();

    if (CompletedSlacks >= TargetSlacks)
    {
        bGameWon = true;
        if (!bGameEnded)
        {
            bGameEnded = true;
            GameEndTime = GetWorld()->GetTimeSeconds();
        }
        ShowWinScreen();
    }
}

void ASlackerGameMode::ShowWinScreen()
{
    HideInteractText();
    StopPenaltyTimer();

    if (!HUDWidget) return;

    UTextBlock* WinText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Win")));
    if (WinText)
    {
        WinText->SetVisibility(ESlateVisibility::Visible);
    }
}

void ASlackerGameMode::ShowInteractText(const FString& Text)
{
    if (!HUDWidget) return;

    if (UTextBlock* InteractText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Interact"))))
    {
        InteractText->SetText(FText::FromString(Text));
        InteractText->SetVisibility(ESlateVisibility::Visible);
    }
}

void ASlackerGameMode::HideInteractText()
{
    if (!HUDWidget) return;

    if (UTextBlock* InteractText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(FName("Text_Interact"))))
    {
        InteractText->SetVisibility(ESlateVisibility::Collapsed);
    }
}