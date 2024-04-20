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
	
	PlayerHUD = Cast<APlayerHUD>(GetHUD()); // GetHUD() is a built-in function of APlayerController
	ServerCheckMatchState();
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
		CooldownTime = GameMode->CooldownTime;

		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AFPSPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (HasAuthority())
	{
		PlayerHUD = (PlayerHUD == nullptr) ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	}
	if (PlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		PlayerHUD->AddAnnouncement();
	}
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
		if(CountdownTime < 0.f)
		{
			PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AFPSPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD && PlayerHUD->Announcement && PlayerHUD->Announcement->WarmupTime;
	if(bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			PlayerHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);
		
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
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
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if(HasAuthority())
	{
		if(FPSGameMode == nullptr)
		{
			FPSGameMode = Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = FPSGameMode->LevelStartingTime;
		}
		FPSGameMode = FPSGameMode == nullptr ? Cast<AFPSGameMode>(UGameplayStatics::GetGameMode(this)) : FPSGameMode;
		if(FPSGameMode)
		{
			SecondsLeft = FMath::CeilToInt(FPSGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	
	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void AFPSPlayerController::PollInit()
{
	if (HasAuthority() && PlayerHUD == nullptr)
	{
		PlayerHUD = (PlayerHUD == nullptr) ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
		if(PlayerHUD && MatchState == MatchState::WaitingToStart && !PlayerHUD->bAnnouncementActive)
		{
			PlayerHUD->AddAnnouncement();
		}
	}
	
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
	else if(MatchState == MatchState::Cooldown) // Cooldown is a custom MatchState I created in FPSGameMode
	{
		HandleCooldown();
	}
}

// For clients
void AFPSPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress) // InProgress pre-exists
		{
			HandleMatchHasStarted();
		}
	else if(MatchState == MatchState::Cooldown) // Cooldown is a custom MatchState I created in FPSGameMode
	{
		HandleCooldown();
	}
}

void AFPSPlayerController::HandleMatchHasStarted()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if(PlayerHUD)
	{
		if(PlayerHUD->CharacterOverlay == nullptr)
		{
			PlayerHUD->AddCharacterOverlay(); // no HUD until game is in-progress
		}
		if(PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AFPSPlayerController::HandleCooldown()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if(PlayerHUD && PlayerHUD->CharacterOverlay)
	{
		PlayerHUD->CharacterOverlay->RemoveFromParent();
		
		bool bHUDValid = PlayerHUD->Announcement &&
			PlayerHUD->Announcement->AnnouncementText &&
				PlayerHUD->Announcement->InfoText;
		
		if(bHUDValid)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New Match Starts In:");
			PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			PlayerHUD->Announcement->InfoText->SetText(FText());
		}
	}
}
