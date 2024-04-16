// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"


UCLASS()
class MULTIPLAYERFPS_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar; // BP progress bar must have same name

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount; // BP text block must have same name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DeathsAmount; // BP text block must have same name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount; // BP text block must have same name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount; // BP text block must have same name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText; // BP text block must have same name
};
