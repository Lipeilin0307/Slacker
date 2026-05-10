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

    UPROPERTY(VisibleAnywhere, Category = "Slack")
    class UStaticMeshComponent* PointMesh;

    UPROPERTY(VisibleAnywhere, Category = "Slack")
    class UBoxComponent* InteractionBox;

    UPROPERTY(BlueprintReadOnly, Category = "Slack")
    float Progress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Slack")
    bool bSlacking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Slack")
    bool bCompleted = false;

    bool bPlayerNearby = false;

    UPROPERTY(EditAnywhere, Category = "Slack")
    float SlackDuration = 5.0f;

    UFUNCTION(BlueprintCallable, Category = "Slack")
    void StartSlack();

    UFUNCTION(BlueprintCallable, Category = "Slack")
    void StopSlack();

    UFUNCTION(BlueprintCallable, Category = "Slack")
    void CompleteSlack();

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void CheckInput();

protected:
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
};