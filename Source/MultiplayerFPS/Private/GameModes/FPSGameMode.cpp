// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/FPSGameMode.h"

#include "Character/FPSCharacter.h"

void AFPSGameMode::PlayerEliminated(AFPSCharacter* ElimmedCharacter, AFPSPlayerController* VictimController,
                                    AFPSPlayerController* AttackerController)
{
	// only on server
	if(ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}
