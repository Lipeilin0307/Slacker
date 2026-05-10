#include "BossAI.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Seat.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

ABossAI::ABossAI()
{
	PrimaryActorTick.bCanEverTick = true;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SetPeripheralVisionAngle(90.0f);
	PawnSensing->SightRadius = 2000.0f;

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ABossAI::BeginPlay()
{
	Super::BeginPlay();
	AIControllerRef = Cast<AAIController>(GetController());

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &ABossAI::OnSeePlayer);
	}
}

void ABossAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AIControllerRef) return;

	switch (CurrentState)
	{
	case EBossState::Patrol:
		PatrolLogic(DeltaTime);
		break;
	case EBossState::Chase:
		ChaseLogic(DeltaTime);
		break;
	case EBossState::Escort:
		EscortLogic(DeltaTime);
		break;
	}

	UpdateBossAnimation();
}

void ABossAI::PatrolLogic(float DeltaTime)
{
	if (PatrolPoints.Num() == 0) return;

	AActor* Target = PatrolPoints[CurrentPatrolIndex];
	if (!Target) return;

	float Dist = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());

	if (Dist < 150.0f)
	{
		CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
	}
	else
	{
		AIControllerRef->MoveToActor(Target, 100.0f, true, true, true);
	}
}

void ABossAI::OnSeePlayer(APawn* Pawn)
{
	if (CurrentState != EBossState::Patrol) return;

	ACharacter* PlayerChar = Cast<ACharacter>(Pawn);
	if (PlayerChar && PlayerSeat.IsValid() && PlayerSeat->bOccupied && PlayerSeat->Occupant == PlayerChar)
	{
		return;
	}

	TargetPlayer = Pawn;
	CurrentState = EBossState::Chase;
	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
}

void ABossAI::ChaseLogic(float DeltaTime)
{
	if (!TargetPlayer)
	{
		ReturnToPatrol();
		return;
	}

	ACharacter* PlayerChar = Cast<ACharacter>(TargetPlayer);
	if (PlayerChar && PlayerSeat.IsValid() && PlayerSeat->bOccupied && PlayerSeat->Occupant == PlayerChar)
	{
		ReturnToPatrol();
		return;
	}

	AIControllerRef->MoveToActor(TargetPlayer, 100.0f, true, true, true);

	float Dist = FVector::Dist2D(GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Dist < 180.0f)
	{
		CurrentState = EBossState::Escort;
		GetCharacterMovement()->MaxWalkSpeed = EscortSpeed;
		AIControllerRef->StopMovement();
	}
}

void ABossAI::EscortLogic(float DeltaTime)
{
	if (!PlayerSeat.IsValid() || !TargetPlayer)
	{
		if (TargetPlayer)
		{
			ACharacter* PlayerChar = Cast<ACharacter>(TargetPlayer);
			if (PlayerChar)
			{
				PlayerChar->SetActorEnableCollision(true);
				if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
				{
					AnimInst->StopAllMontages(0.25f);
				}
				PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
			}
		}
		ReturnToPatrol();
		return;
	}

	ASeat* Seat = PlayerSeat.Get();
	ACharacter* PlayerChar = Cast<ACharacter>(TargetPlayer);

	if (PlayerChar)
	{
		PlayerChar->GetCharacterMovement()->SetMovementMode(MOVE_None);
		PlayerChar->SetActorEnableCollision(false);

		// Play being-carried animation on player
		if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
		{
			if (CarriedMontage && !AnimInst->Montage_IsPlaying(CarriedMontage))
			{
				AnimInst->Montage_Play(CarriedMontage);
			}
		}
	}

	float DistToSeat = FVector::Dist2D(GetActorLocation(), Seat->GetActorLocation());

	if (DistToSeat < 200.0f)
	{
		if (PlayerChar)
		{
			// Must stop carried montage BEFORE ForceSit/Sit plays sit montage
			if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
			{
				AnimInst->StopAllMontages(0.0f);
			}
			PlayerChar->SetActorEnableCollision(true);
			Seat->ForceSit(PlayerChar);
		}

		CurrentState = EBossState::Patrol;
		TargetPlayer = nullptr;
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
		if (AIControllerRef)
		{
			AIControllerRef->StopMovement();
		}
		return;
	}

	FVector Dir = Seat->GetActorLocation() - GetActorLocation();
	Dir.Z = 0.0f;
	Dir.Normalize();

	AddMovementInput(Dir);

	if (PlayerChar)
	{
		FVector CarryPos = GetActorLocation() + GetActorForwardVector() * 60.f;
		CarryPos.Z += 80.0f;
		PlayerChar->SetActorLocation(CarryPos);
	}
}

void ABossAI::ReturnToPatrol()
{
	CurrentState = EBossState::Patrol;
	TargetPlayer = nullptr;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	if (AIControllerRef)
	{
		AIControllerRef->StopMovement();
	}
}

void ABossAI::UpdateBossAnimation()
{
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	UAnimMontage* TargetMontage = nullptr;
	switch (CurrentState)
	{
	case EBossState::Patrol:
		TargetMontage = WalkMontage;
		break;
	case EBossState::Chase:
		TargetMontage = RunMontage;
		break;
	case EBossState::Escort:
		TargetMontage = CarryMontage;
		break;
	}

	if (TargetMontage && !AnimInst->Montage_IsPlaying(TargetMontage))
	{
		AnimInst->Montage_Play(TargetMontage);
	}
}