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
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void OnPossess(APawn* InPawn) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	class APlayerHUD* PlayerHUD;
	
};
