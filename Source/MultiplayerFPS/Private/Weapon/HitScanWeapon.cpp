// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Character/FPSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Weapon/WeaponTypes.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		AFPSCharacter* HitPlayer = Cast<AFPSCharacter>(FireHit.GetActor());
		
		if(HitPlayer)
		{
			FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(Start, FireHit.ImpactPoint);
			GetRagdollInfo(FireHit, HitPlayer, Direction);
		}
		
		if(HitPlayer && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(
				HitPlayer,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
		}
			
		if(ImpactParticles)
			{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
				);
			}
	
		if(HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			FireHit.ImpactPoint
			);
		}
	
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
				);
		}
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
					this,
					FireSound,
					GetActorLocation()
				);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
		
		World->LineTraceSingleByChannel(
				OutHit,
				TraceStart,
				End,
				ECC_Visibility
			);
		
		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if(BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AHitScanWeapon::GetRagdollInfo(const FHitResult& FireHit, AFPSCharacter* HitPlayer, const FVector& Direction)
{
	if(HitPlayer->Implements<URagdollInterface>())
	{
		USkeletalMeshComponent* HitPlayerMesh = HitPlayer->GetMesh();
		FName HitBone = HitPlayerMesh->FindClosestBone(FireHit.ImpactPoint);
		FName ClosestBoneName;
		int32 ListNum = 1;
		for(const FName& BoneName : BoneNames)
		{
			if(HitPlayerMesh->BoneIsChildOf(HitBone, BoneName) || HitBone == BoneName)
			{
				ClosestBoneName = BoneName;
				Execute_GetRagdollInfo(HitPlayer, ClosestBoneName, Direction);
				break;
			}
			if(ListNum == BoneNames.Num())
			{
				ClosestBoneName = FName("spine_04");
				Execute_GetRagdollInfo(HitPlayer, ClosestBoneName, Direction);
				break;
			}
			ListNum++;
		}
	}
}


FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	// TO VISUALIZE SCATTER
	
	/*DrawDebugSphere(
		GetWorld(),
		SphereCenter,
		SphereRadius,
		12,
		FColor::Red,
		true
		);

	DrawDebugSphere(
		GetWorld(),
		EndLoc,
		4.f,
		12,
		FColor::Orange,
		true
		);

	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true
		);*/

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()); // TRACE_LENGTH from macro in WeaponTypes.h
}
