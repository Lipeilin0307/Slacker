#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameModeBase.h"
#include "SlackerGameMode.generated.h"

UCLASS()
class SLACKER_API ASlackerGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASlackerGameMode();

    UPROPERTY(BlueprintReadOnly, Category = "GameState")
    int32 CompletedSlacks = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameState")
    int32 TargetSlacks = 3;

    UPROPERTY(BlueprintReadOnly, Category = "GameState")
    bool bGameWon = false;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> HUDWidgetClass;

    UPROPERTY()
    UUserWidget* HUDWidget;

    UFUNCTION(BlueprintCallable, Category = "GameState")
    void OnSlackCompleted();

    UFUNCTION(BlueprintCallable, Category = "GameState")
    void ShowWinScreen();

    void UpdateHUD();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowInteractText(const FString& Text);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideInteractText();

    UFUNCTION(BlueprintCallable, Category = "Penalty")
    void StartPenaltyTimer();

    UFUNCTION(BlueprintCallable, Category = "Penalty")
    void StopPenaltyTimer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float GameStartTime = 0.0f;
    float GameEndTime = 0.0f;
    float PenaltyStartTime = 0.0f;
    bool bGameEnded = false;
    bool bPenaltyActive = false;

    void UpdateTimerDisplay();
    void UpdatePenaltyDisplay();
};