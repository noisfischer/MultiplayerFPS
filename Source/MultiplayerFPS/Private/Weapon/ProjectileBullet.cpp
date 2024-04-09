// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if(OwnerController)
		{
			if(OtherActor->Implements<URagdollInterface>())
			{
				USkeletalMeshComponent* HitPlayerMesh = Cast<USkeletalMeshComponent>(Hit.Component.Get());
				FName HitBone = HitPlayerMesh->FindClosestBone(Hit.Location);
				FName ClosestBoneName;
				int32 ListNum = 1;
				for(const FName& BoneName : BoneNames)
				{
					if(HitPlayerMesh->BoneIsChildOf(HitBone, BoneName) || HitBone == BoneName)
					{
						ClosestBoneName = BoneName;
						Execute_GetRagdollInfo(OtherActor, ClosestBoneName, GetActorForwardVector());
						break;
					}
					if(ListNum == BoneNames.Num())
					{
						ClosestBoneName = FName("spine_04");
						Execute_GetRagdollInfo(OtherActor, ClosestBoneName, GetActorForwardVector());
						break;
					}
					ListNum++;
				}
			}
			
			UGameplayStatics::ApplyDamage(
				OtherActor,
				Damage,
				OwnerController,
				this,
				UDamageType::StaticClass()
				);
		}
		
		Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	}
}
