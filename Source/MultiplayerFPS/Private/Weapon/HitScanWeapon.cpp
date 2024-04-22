// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"

#include "Character/FPSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

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
		FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(Start, End);

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
					GetRagdollInfo(FireHit, HitPlayer, Direction);
					
					if(HasAuthority())
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
							World,
							ImpactParticles,
							FireHit.ImpactPoint,
							FireHit.ImpactNormal.Rotation()
						);
					}
				}
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
