#include "SlackPoint.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ProgressBar.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "SlackerGameMode.h"

ASlackPoint::ASlackPoint()
{
    PrimaryActorTick.bCanEverTick = true;

    PointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PointMesh"));
    RootComponent = PointMesh;

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetCollisionProfileName(TEXT("Trigger"));
    InteractionBox->SetBoxExtent(FVector(100, 100, 100));
}

void ASlackPoint::BeginPlay()
{
    Super::BeginPlay();

    InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ASlackPoint::OnOverlapBegin);
    InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ASlackPoint::OnOverlapEnd);

    // Hide the slack progress bar at game start
    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
    if (GM && GM->HUDWidget)
    {
        if (UProgressBar* ProgressBar = Cast<UProgressBar>(GM->HUDWidget->GetWidgetFromName(FName("Progress_Slack"))))
        {
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ASlackPoint::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckInput();

    if (bSlacking && !bCompleted && bPlayerNearby)
    {
        Progress += DeltaTime / SlackDuration;

        ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
        if (GM && GM->HUDWidget)
        {
            UProgressBar* ProgressBar = Cast<UProgressBar>(GM->HUDWidget->GetWidgetFromName(FName("Progress_Slack")));
            if (ProgressBar)
            {
                ProgressBar->SetPercent(Progress);
            }
        }

        if (Progress >= 1.0f)
        {
            CompleteSlack();
        }
    }
}

void ASlackPoint::CheckInput()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    if (PC->WasInputKeyJustPressed(EKeys::E))
    {
        if (bPlayerNearby && !bCompleted)
        {
            if (!bSlacking)
            {
                StartSlack();
            }
            else
            {
                StopSlack();
            }
        }
    }
}

void ASlackPoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        bPlayerNearby = true;
        if (!bCompleted)
        {
            ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
            if (GM) GM->ShowInteractText(TEXT("Press E to Slack"));
        }
    }
}

void ASlackPoint::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharacter* Char = Cast<ACharacter>(OtherActor);
    if (Char && Char->IsPlayerControlled())
    {
        bPlayerNearby = false;

        ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
        if (GM) GM->HideInteractText();

        StopSlack();
    }
}

void ASlackPoint::StartSlack()
{
    bSlacking = true;
    Progress = 0.0f;

    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
    if (GM && GM->HUDWidget)
    {
        UProgressBar* ProgressBar = Cast<UProgressBar>(GM->HUDWidget->GetWidgetFromName(FName("Progress_Slack")));
        if (ProgressBar)
        {
            ProgressBar->SetPercent(0.0f);
            ProgressBar->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void ASlackPoint::StopSlack()
{
    bSlacking = false;
    Progress = 0.0f;

    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
    if (GM && GM->HUDWidget)
    {
        UProgressBar* ProgressBar = Cast<UProgressBar>(GM->HUDWidget->GetWidgetFromName(FName("Progress_Slack")));
        if (ProgressBar)
        {
            ProgressBar->SetPercent(0.0f);
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ASlackPoint::CompleteSlack()
{
    bSlacking = false;
    bCompleted = true;
    Progress = 1.0f;

    ASlackerGameMode* GM = Cast<ASlackerGameMode>(GetWorld()->GetAuthGameMode());
    if (GM && GM->HUDWidget)
    {
        UProgressBar* ProgressBar = Cast<UProgressBar>(GM->HUDWidget->GetWidgetFromName(FName("Progress_Slack")));
        if (ProgressBar)
        {
            ProgressBar->SetPercent(1.0f);
            ProgressBar->SetVisibility(ESlateVisibility::Collapsed);
        }

        GM->OnSlackCompleted();
    }
}