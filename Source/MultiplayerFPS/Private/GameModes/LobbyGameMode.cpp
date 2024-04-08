// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

// Keeps track of incoming players
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Gets number of players that are in the game
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);
		if(NumberOfPlayers == 2)
		{
			UWorld* World = GetWorld();
			if(World)
			{
				bUseSeamlessTravel = true;	// Needed so that client doesn't need to disconnect
				World->ServerTravel("/Game/Maps/FPSMap?listen");
			}
		}
	}
}
