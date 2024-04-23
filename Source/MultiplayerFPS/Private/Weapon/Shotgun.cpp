// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"

#include "Character/FPSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<AFPSCharacter*, uint32> HitMap;
		
		for(uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			AFPSCharacter* HitPlayer = Cast<AFPSCharacter>(FireHit.GetActor());
			if(HitPlayer && HasAuthority() && InstigatorController)
			{
				FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(Start, FireHit.ImpactPoint);
				GetRagdollInfo(FireHit, HitPlayer, Direction);
			
				if(HitMap.Contains(HitPlayer))
				{
					HitMap[HitPlayer]++;
				}
				else
				{
					HitMap.Emplace(HitPlayer, 1);
				}
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
				FireHit.ImpactPoint,
				.5f,
				FMath::FRandRange(-.5f, .5f)
				);
			}
		}

		for(auto HitPair : HitMap)
		{
			if(HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
				HitPair.Key,
				Damage * HitPair.Value,
				InstigatorController,
				this,
				UDamageType::StaticClass()
				);
			}
		}
	}
}
