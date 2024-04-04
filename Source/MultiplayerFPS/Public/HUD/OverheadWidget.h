// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"


UCLASS()
class MULTIPLAYERFPS_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) // any changes we make in C++ will be applied to the widget BP
	class UTextBlock* DisplayText; // text block in child BP must have same name 'DisplayText' and must re-parent to this class type

	void SetDisplayText(FString TextToDisplay);
	void ShowPlayerNetRole(APawn* InPawn);
	
protected:
	virtual void NativeDestruct() override;
};
