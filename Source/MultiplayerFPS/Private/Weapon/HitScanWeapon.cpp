// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Character/FPSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);
			if(FireHit.bBlockingHit)
			{
				AFPSCharacter* HitPlayer = Cast<AFPSCharacter>(FireHit.GetActor());
				if(HitPlayer)
				{
					UGameplayStatics::ApplyDamage(
						HitPlayer,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);

					if(ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(
							World,
							ImpactParticles,
							End,
							FireHit.ImpactNormal.Rotation()
						);
					}
				}
			}
		}
	}
}
