// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileBullet.generated.h"


UCLASS()
class MULTIPLAYERFPS_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileBullet();

private:
	TArray<FName> BoneNames {
		"upperarm_r",
		"upperarm_l",
		"neck_01",
		"thigh_l",
		"thigh_r",
	};
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	
};
