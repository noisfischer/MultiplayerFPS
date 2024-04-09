// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "FPSGameMode.generated.h"


UCLASS()
class MULTIPLAYERFPS_API AFPSGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class AFPSCharacter* ElimmedCharacter, class AFPSPlayerController* VictimController, AFPSPlayerController* AttackerController);
	
};
