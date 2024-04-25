// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Character/FPSCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "MultiplayerFPS/MultiplayerFPS.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates =  true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block); // ECC_SkeletalMesh is GameTraceChannel1. We overrode this in project settings and redefined it as ECC_SkeletalMesh in MultiplayerFPS.h
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if(Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if(HasAuthority()) // if server
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& Hit)
{
		Destroy();
}

void AProjectile::SpawnTrailSystem()
{
	if(TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
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


void AProjectile::Multicast_RagdollBlast_Implementation()
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

void AProjectile::ExplodeDamage()
{
	Multicast_RagdollBlast();

	/*
	 *Apply Radial Damage
	 */
	
	// GetInstigator returns the pawn that fired the rocket
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.f,
				GetActorLocation(),
				DamageInnerRadius,
				DamageOuterRadius,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(), // just an empty array for ignore actors
				this,
				FiringController // instigator controller
			);
		}
	}
}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
	DestroyTimer,
	this,
	&AProjectile::DestroyTimerFinished,
	DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if(ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticles,
			GetActorTransform()
			// other inputs optional
		);
	}
	if(ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
			// other inputs optional
		);
	}
}

