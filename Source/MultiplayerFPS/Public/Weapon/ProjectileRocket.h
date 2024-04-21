// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "Interfaces/RagdollInterface.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERFPS_API AProjectileRocket : public AProjectile, public IRagdollInterface
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	void DestroyTimerFinished();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RagdollBlast();

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;
	
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

};
