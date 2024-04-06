// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletCasing.generated.h"

UCLASS()
class MULTIPLAYERFPS_API ABulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletCasing();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

protected:
	virtual void BeginPlay() override;

};
