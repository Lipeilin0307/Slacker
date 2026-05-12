#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlackPoint.generated.h"

UCLASS()
class SLACKER_API ASlackPoint : public AActor
{
    GENERATED_BODY()

public:
    ASlackPoint();

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, Category = "SlackPoint")
    class UStaticMeshComponent* PointMesh;

    UPROPERTY(VisibleAnywhere, Category = "SlackPoint")
    class UBoxComponent* InteractionBox;

    UPROPERTY(EditAnywhere, Category = "SlackPoint")
    float SlackDuration = 3.0f;

    UPROPERTY(BlueprintReadOnly, Category = "SlackPoint")
    bool bSlacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "SlackPoint")
    bool bCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "SlackPoint")
    bool bPlayerNearby = false;

    float Progress = 0.0f;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void StartSlack();
    void StopSlack();
    void CompleteSlack();
    void CheckInput();

private:
    void ShowInteractPrompt();
    void HideInteractPrompt();
};