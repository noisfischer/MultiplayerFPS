// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/FPSGameMode.h"

#include "Character/FPSCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/FPSPlayerController.h"
#include "PlayerState/FPSPlayerState.h"

class UEnhancedInputLocalPlayerSubsystem;

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AFPSGameMode::AFPSGameMode()
{
	bDelayedStart = true; // Game mode will stay in "Waiting To Start" state until otherwise told
}

void AFPSGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AFPSGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
}

void AFPSGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	
	// Loops through all player controllers in the game
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AFPSPlayerController* FPSPlayer = Cast<AFPSPlayerController>(*It);
		if(FPSPlayer)
		{
			FPSPlayer->OnMatchStateSet(MatchState); // sets the match state on each player controller
		}
	}
}

void AFPSGameMode::PlayerEliminated(AFPSCharacter* ElimmedCharacter, AFPSPlayerController* VictimController,
                                    AFPSPlayerController* AttackerController)
{
	AFPSPlayerState* AttackerPlayerState = AttackerController ? Cast<AFPSPlayerState>(AttackerController->PlayerState) : nullptr;
	AFPSPlayerState* VictimPlayerState = VictimController ? Cast<AFPSPlayerState>(VictimController->PlayerState) : nullptr;

	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}
	
	// only on server
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void AFPSGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Reset();	// pre-existing function that detaches the character from the controller
		ElimmedCharacter->Destroy(); // the controller still exists though
	}
	if(ElimmedController)
	{
		// Get all Player Start actors in the level
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1); // random player start spawn

		// pre-existing function that spawns a new character at a start spot
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
