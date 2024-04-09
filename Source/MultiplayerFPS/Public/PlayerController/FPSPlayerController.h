// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"


UCLASS()
class MULTIPLAYERFPS_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	
protected:
	virtual void BeginPlay() override;
	
private:
	class APlayerHUD* PlayerHUD;
	
};
