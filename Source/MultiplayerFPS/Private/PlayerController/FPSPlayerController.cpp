// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FPSPlayerController.h"

#include "Character/FPSCharacter.h"
#include "Components/ProgressBar.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/PlayerHUD.h"

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD()); // GetHUD() is a built-in function of APlayerController
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
