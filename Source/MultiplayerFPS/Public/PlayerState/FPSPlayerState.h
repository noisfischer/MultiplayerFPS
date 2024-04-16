// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FPSPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERFPS_API AFPSPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// same as OnRep_Score, but for Server
	void AddToScore(float ScoreAmount);
	// For clients only. Pre-existing in APlayerState
    virtual void OnRep_Score() override;
private:
	class AFPSCharacter* PlayerRef;
	class AFPSPlayerController* Controller;
};
