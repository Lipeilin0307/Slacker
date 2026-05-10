#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimMontage.h"
#include "Seat.generated.h"

UCLASS()
class SLACKER_API ASeat : public AActor
{
    GENERATED_BODY()

public:
    ASeat();

    UPROPERTY(VisibleAnywhere, Category = "Seat")
    class UStaticMeshComponent* SeatMesh;

    UPROPERTY(VisibleAnywhere, Category = "Seat")
    class UBoxComponent* InteractionBox;

    UPROPERTY(EditAnywhere, Category = "Seat")
    FVector SitOffset = FVector(0, 0, 100);

    UPROPERTY(BlueprintReadOnly, Category = "Seat")
    bool bOccupied = false;

    UPROPERTY(BlueprintReadOnly, Category = "Seat")
    bool bForceLocked = false;

    UPROPERTY(BlueprintReadOnly, Category = "Seat")
    class ACharacter* Occupant = nullptr;

    bool bPlayerNearby = false;

    UPROPERTY(EditAnywhere, Category = "Animation")
    UAnimMontage* SitMontage;

    UFUNCTION(BlueprintCallable, Category = "Seat")
    void Sit(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Seat")
    void StandUp();

    UFUNCTION(BlueprintCallable, Category = "Seat")
    void ForceSit(ACharacter* Character);

    UFUNCTION()
    void OnInteractPressed();

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void AutoStandUp();

protected:
    virtual void BeginPlay() override;
};