// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(TrailSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void AProjectileRocket::Multicast_RagdollBlast_Implementation()
{
	/*
	 * FOR RAGDOLL ON DEATH
	 */
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;
	ECollisionChannel CollisionChannel = ECC_Pawn;
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		GetActorLocation(),
		GetActorLocation(),
		FQuat::Identity,
		CollisionChannel,
		FCollisionShape::MakeSphere(500.f),
		QueryParams
	);

	if(bHit)
	{
		for(auto HitPlayer : HitResults)
		{
			if(HitPlayer.GetActor()->Implements<URagdollInterface>())
			{
				FVector PlayerLocation = HitPlayer.GetActor()->GetActorLocation();
				FVector RagdollDirection = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), PlayerLocation);
	
				Execute_GetRagdollInfo(HitPlayer.GetActor(), FName("spine_03"), RagdollDirection);
			}
		}
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	Multicast_RagdollBlast();

	/*
	 *Apply Radial Damage
	 */
	
	// GetInstigator returns the pawn that fired the rocket
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				200.f,
				500.f,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(), // just an empty array for ignore actors
				this,
				FiringController // instigator controller
			);
		}
	}
	
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileRocket::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}
