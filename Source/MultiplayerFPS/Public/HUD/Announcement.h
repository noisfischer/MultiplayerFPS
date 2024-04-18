// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

UCLASS()
class MULTIPLAYERFPS_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmupTime; // BP text block must have same name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText; // BP text block must have same name

	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText; // BP text block must have same name
};
