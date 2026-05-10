#include "Seat.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "SlackerGameMode.h"

ASeat::ASeat()
{
    PrimaryActorTick.bCanEverTick = false;

    SeatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SeatMesh"));
    RootComponent = SeatMesh;

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
    InteractionBox->SetBoxExtent(FVector(100, 100, 100));
}

void ASeat::BeginPlay()
{
    Super::BeginPlay();

    InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ASeat::OnOverlapBegin);
    InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ASeat::OnOverlapEnd);

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        EnableInput(PC);
        InputComponent->BindAction("Interact", IE_Pressed, this, &ASeat::OnInteractPressed);
    }
}

void ASeat::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        bPlayerNearby = true;

        ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
        if (GM)
        {
            if (!bOccupied)
                GM->ShowInteractText(TEXT("Press E to Sit"));
            else if (Occupant == Char && !bForceLocked)
                GM->ShowInteractText(TEXT("Press E to Stand"));
        }
    }
}

void ASeat::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        bPlayerNearby = false;

        ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
        if (GM) GM->HideInteractText();
    }
}

void ASeat::OnInteractPressed()
{
    if (!bPlayerNearby) return;
    if (bForceLocked) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    ACharacter* PlayerChar = Cast<ACharacter>(PC->GetPawn());
    if (!PlayerChar) return;

    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());

    if (!bOccupied)
    {
        Sit(PlayerChar);
        if (GM) GM->ShowInteractText(TEXT("Press E to Stand"));
    }
    else if (Occupant == PlayerChar)
    {
        StandUp();
        if (GM) GM->ShowInteractText(TEXT("Press E to Sit"));
    }
}

void ASeat::Sit(ACharacter* Character)
{
    if (!Character || bOccupied) return;

    bOccupied = true;
    Occupant = Character;
    Character->GetCharacterMovement()->SetMovementMode(MOVE_None);
    Character->SetActorLocation(GetActorLocation() + SitOffset);

    // Play sitting animation (stop any ongoing montage first to prevent blending with carried animation)
    if (UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance())
    {
        AnimInst->StopAllMontages(0.0f); // stop immediately, no blending
        if (SitMontage)
        {
            AnimInst->Montage_Play(SitMontage);
            UE_LOG(LogTemp, Warning, TEXT("Playing SitMontage (forced stop previous montages)"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("SitMontage is NULL! Assign AM_Player_Sit in editor."));
        }
    }
}

void ASeat::StandUp()
{
    if (!Occupant) return;

    // Stop all montages (sitting / being carried)
    if (UAnimInstance* AnimInst = Occupant->GetMesh()->GetAnimInstance())
    {
        AnimInst->StopAllMontages(0.25f);
    }

    Occupant->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    FVector StandPos = Occupant->GetActorLocation() + Occupant->GetActorForwardVector() * 100.0f;
    Occupant->SetActorLocation(StandPos);

    bOccupied = false;
    Occupant = nullptr;
}

void ASeat::ForceSit(ACharacter* Character)
{
    Sit(Character);

    bForceLocked = true;

    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
    if (GM)
    {
        GM->HideInteractText();
        GM->StartPenaltyTimer();
    }

    FTimerHandle TimerHandle;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &ASeat::AutoStandUp);
    GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 3.0f, false);
}

void ASeat::AutoStandUp()
{
    StandUp();
    bForceLocked = false;

    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
    if (GM) GM->StopPenaltyTimer();

    if (bPlayerNearby)
    {
        if (GM) GM->ShowInteractText(TEXT("Press E to Sit"));
    }
}