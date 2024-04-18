// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FPSPlayerController.h"

#include "Character/FPSCharacter.h"
#include "Components/ProgressBar.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/PlayerHUD.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameModes/FPSGameMode.h"
#include "HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ServerCheckMatchState();

	PlayerHUD = Cast<APlayerHUD>(GetHUD()); // GetHUD() is a built-in function of APlayerController
}

void AFPSPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void AFPSPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AFPSPlayerController::ServerCheckMatchState_Implementation()
{
	AFPSGameMode* GameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		LevelStartingTime = GameMode->LevelStartingTime;
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		MatchState = GameMode->GetMatchState();

		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime);

		if(PlayerHUD && MatchState == MatchState::WaitingToStart)
		{
			PlayerHUD->AddAnnouncement();
		}
	}
}

void AFPSPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
}

void AFPSPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSPlayerController, MatchState);
}

void AFPSPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	// If PlayerHUD is null, set it correctly. If it's already set, leave it as PlayerHUD
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->HealthBar;
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AFPSPlayerController::SetHUDScore(float Score)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->ScoreAmount;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PlayerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AFPSPlayerController::SetHUDDeaths(int32 Deaths)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->DeathsAmount;
	if(bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		PlayerHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDeaths = Deaths;
	}
}

void AFPSPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->WeaponAmmoAmount;
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AFPSPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->CarriedAmmoAmount;
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AFPSPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD && PlayerHUD->CharacterOverlay && PlayerHUD->CharacterOverlay->MatchCountdownText;
	if(bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFPSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(InPawn);
	if(FPSCharacter)
	{
		SetHUDHealth(FPSCharacter->GetHealth(), FPSCharacter->GetMaxHealth());
	}
}

void AFPSPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if(CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void AFPSPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(PlayerHUD && PlayerHUD->CharacterOverlay)
		{
			CharacterOverlay = PlayerHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDeaths(HUDDeaths);
			}
		}
	}
}

void AFPSPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AFPSPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest; // time elapse since the client sent the RPC
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AFPSPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AFPSPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if(IsLocalController()) // for clients
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

// For server
void AFPSPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if(MatchState == MatchState::InProgress) // InProgress pre-exists
	{
		HandleMatchHasStarted();
	}
}

// For clients
void AFPSPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress) // InProgress pre-exists
		{
			HandleMatchHasStarted();
		}
}

void AFPSPlayerController::HandleMatchHasStarted()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if(PlayerHUD)
	{
		PlayerHUD->AddCharacterOverlay(); // no HUD until game is in-progress
		if(PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
