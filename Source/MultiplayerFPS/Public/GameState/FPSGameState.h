// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "FPSGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERFPS_API AFPSGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AFPSPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AFPSPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
	
};
