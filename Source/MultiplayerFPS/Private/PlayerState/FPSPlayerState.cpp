// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/FPSPlayerState.h"

#include "Character/FPSCharacter.h"
#include "PlayerController/FPSPlayerController.h"
#include "Net/UnrealNetwork.h"

void AFPSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSPlayerState, Deaths);
}

void AFPSPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount); // SetScore and GetScore pre-exist in APlayerState
	PlayerRef = PlayerRef == nullptr ? Cast<AFPSCharacter>(GetPawn()) : PlayerRef;
	if(PlayerRef)
	{
		Controller = Controller == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AFPSPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	PlayerRef = PlayerRef == nullptr ? Cast<AFPSCharacter>(GetPawn()) : PlayerRef;
	if(PlayerRef)
	{
		Controller = Controller == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AFPSPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;
	PlayerRef = PlayerRef == nullptr ? Cast<AFPSCharacter>(GetPawn()) : PlayerRef;
	if(PlayerRef)
	{
		Controller = Controller == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		
		}
	}
}

void AFPSPlayerState::OnRep_Deaths()
{
	PlayerRef = PlayerRef == nullptr ? Cast<AFPSCharacter>(GetPawn()) : PlayerRef;
	if(PlayerRef)
	{
		Controller = Controller == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}
