// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"


UCLASS()
class MULTIPLAYERFPS_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override; // from AHUD base class
};