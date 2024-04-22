// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "Interfaces/RagdollInterface.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERFPS_API AHitScanWeapon : public AWeapon, public IRagdollInterface
{
	GENERATED_BODY()

public:
    virtual void Fire(const FVector& HitTarget);
	void GetRagdollInfo(const FHitResult& FireHit, AFPSCharacter* HitPlayer, const FVector& Direction);

private:
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	TArray<FName> BoneNames {
		"upperarm_r",
		"upperarm_l",
		"neck_01",
		"thigh_l",
		"thigh_r",
	};
};
