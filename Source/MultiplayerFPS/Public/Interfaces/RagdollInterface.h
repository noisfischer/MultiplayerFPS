// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RagdollInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URagdollInterface : public UInterface
{
	GENERATED_BODY()
};


class MULTIPLAYERFPS_API IRagdollInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetRagdollInfo(const FName& BoneName, const FVector& ImpulseDirection);
};
