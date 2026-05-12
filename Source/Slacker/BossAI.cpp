#include "BossAI.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "GameFramework/PlayerController.h"
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

	EscortEnterTime = -99999.0f;
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
	UpdatePlayerWalkAnimation();
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

	if (PlayerChar)
	{
		PlayerChar->GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
		if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
		{
			if (PlayerRunMontage && !AnimInst->Montage_IsPlaying(PlayerRunMontage))
			{
				AnimInst->Montage_Play(PlayerRunMontage);
			}
		}
	}
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

	if (PlayerChar)
	{
		if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
		{
			if (PlayerRunMontage && !AnimInst->Montage_IsPlaying(PlayerRunMontage))
			{
				AnimInst->Montage_Play(PlayerRunMontage);
			}
		}
	}

	float Dist = FVector::Dist2D(GetActorLocation(), TargetPlayer->GetActorLocation());
	if (Dist < 180.0f)
	{
		CurrentState = EBossState::Escort;
		GetCharacterMovement()->MaxWalkSpeed = EscortSpeed;
		EscortEnterTime = GetWorld()->GetTimeSeconds();

		if (PlayerChar && PlayerRunMontage)
		{
			if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
			{
				AnimInst->Montage_Stop(0.25f, PlayerRunMontage);
			}
		}
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

		if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
		{
			if (CarriedMontage && !AnimInst->Montage_IsPlaying(CarriedMontage))
			{
				AnimInst->Montage_Play(CarriedMontage);
			}
		}
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - EscortEnterTime > 10.0f)
	{
		if (PlayerChar)
		{
			PlayerChar->SetActorEnableCollision(true);
			if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
			{
				AnimInst->StopAllMontages(0.0f);
			}
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

	float DistToSeat = FVector::Dist2D(GetActorLocation(), Seat->GetActorLocation());
	if (DistToSeat < 200.0f)
	{
		if (PlayerChar)
		{
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
	if (TargetPlayer)
	{
		ACharacter* PlayerChar = Cast<ACharacter>(TargetPlayer);
		if (PlayerChar && PlayerRunMontage)
		{
			if (UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance())
			{
				AnimInst->Montage_Stop(0.25f, PlayerRunMontage);
			}
		}
	}

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

void ABossAI::UpdatePlayerWalkAnimation()
{
	if (CurrentState == EBossState::Chase || CurrentState == EBossState::Escort)
		return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
	if (!PlayerChar || !PlayerWalkMontage) return;

	if (PlayerChar->GetCharacterMovement()->MovementMode == MOVE_None)
		return;

	UAnimInstance* AnimInst = PlayerChar->GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	FVector Velocity = PlayerChar->GetVelocity();
	Velocity.Z = 0.0f;
	bool bIsMoving = Velocity.Size() > 10.0f;

	if (bIsMoving)
	{
		if (!AnimInst->Montage_IsPlaying(PlayerWalkMontage))
		{
			AnimInst->Montage_Play(PlayerWalkMontage);
		}
	}
	else if (AnimInst->Montage_IsPlaying(PlayerWalkMontage))
	{
		AnimInst->Montage_Stop(0.25f, PlayerWalkMontage);
	}
}