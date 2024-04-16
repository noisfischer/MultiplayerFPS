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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void AddToScore(float ScoreAmount); // same as OnRep_Score, but for Server
	void AddToDeaths(int32 DeathsAmount);

    virtual void OnRep_Score() override;	// For clients only. Pre-existing in APlayerState

	UFUNCTION() // Rep Notifies MUST BE UFUNCTIONs
	virtual void OnRep_Deaths();

private:
	class AFPSCharacter* PlayerRef;
	class AFPSPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
};
