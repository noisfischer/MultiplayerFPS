// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/FPSPlayerController.h"
#include "HUD/PlayerHUD.h"

void AFPSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD()); // GetHUD() is a built-in function of APlayerController
}
