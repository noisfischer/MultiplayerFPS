// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

// Keeps track of incoming players
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Gets number of players that are in the game
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	if(NumberOfPlayers == 2)
	{
	//	UWorld::ServerTravel()
	}
}
