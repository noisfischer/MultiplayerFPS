// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/FPSGameMode.h"

#include "Character/FPSCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerController/FPSPlayerController.h"
#include "PlayerState/FPSPlayerState.h"

void AFPSGameMode::PlayerEliminated(AFPSCharacter* ElimmedCharacter, AFPSPlayerController* VictimController,
                                    AFPSPlayerController* AttackerController)
{
	AFPSPlayerState* AttackerPlayerState = AttackerController ? Cast<AFPSPlayerState>(AttackerController->PlayerState) : nullptr;
	AFPSPlayerState* VictimPlayerState = VictimController ? Cast<AFPSPlayerState>(VictimController->PlayerState) : nullptr;

	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
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
