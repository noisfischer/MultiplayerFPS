// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/FPSGameState.h"

#include "Net/UnrealNetwork.h"
#include "PlayerState/FPSPlayerState.h"

void AFPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSGameState, TopScoringPlayers);
}

void AFPSGameState::UpdateTopScore(class AFPSPlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if(ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);

		TopScore = ScoringPlayer->GetScore();
	}
}
