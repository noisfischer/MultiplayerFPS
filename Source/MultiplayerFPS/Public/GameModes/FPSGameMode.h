// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPSGameMode.generated.h"

namespace MatchState
{
	// Create a custom match state - match duration has been reached. Display winner and begin cooldown timer.
	extern MULTIPLAYERFPS_API const FName Cooldown; 
}

UCLASS()
class MULTIPLAYERFPS_API AFPSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFPSGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(class AFPSCharacter* ElimmedCharacter, class AFPSPlayerController* VictimController, AFPSPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
