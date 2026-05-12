#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossAI.generated.h"

class ASeat;
class AAIController;

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Patrol  UMETA(DisplayName = "Patrol"),
	Chase   UMETA(DisplayName = "Chase"),
	Escort  UMETA(DisplayName = "Escort")
};

UCLASS()
class SLACKER_API ABossAI : public ACharacter
{
	GENERATED_BODY()

public:
	ABossAI();

	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	TArray<AActor*> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float PatrolSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Chase")
	float ChaseSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Escort")
	float EscortSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Penalty")
	TWeakObjectPtr<ASeat> PlayerSeat;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EBossState CurrentState = EBossState::Patrol;

	int32 CurrentPatrolIndex = 0;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnSeePlayer(APawn* Pawn);

private:
	UPROPERTY()
	AAIController* AIControllerRef;

	UPROPERTY()
	APawn* TargetPlayer;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* WalkMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* RunMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* CarryMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* CarriedMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* PlayerRunMontage;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* PlayerWalkMontage;

	float LastEscortMoveTime = 0.0f;
	float EscortEnterTime = -99999.0f;

	void PatrolLogic(float DeltaTime);
	void ChaseLogic(float DeltaTime);
	void EscortLogic(float DeltaTime);
	void ReturnToPatrol();
	void UpdateBossAnimation();
	void UpdatePlayerWalkAnimation();
};