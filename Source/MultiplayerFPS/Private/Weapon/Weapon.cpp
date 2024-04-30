// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/FPSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Weapon/BulletCasing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PlayerController/FPSPlayerController.h"
#include "Components/CombatComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // start with no collision

	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		
		// Bind to functions
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereBeginOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if(PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register all replicated variables here
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacter* PlayerRef = Cast<AFPSCharacter>(OtherActor);
	if(PlayerRef)
	{
		PlayerRef->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSCharacter* PlayerRef = Cast<AFPSCharacter>(OtherActor);
	if(PlayerRef)
	{
		PlayerRef->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	FPSOwnerPlayer = FPSOwnerPlayer == nullptr ? Cast<AFPSCharacter>(GetOwner()) : FPSOwnerPlayer;
	if(FPSOwnerPlayer)
	{
		FPSOwnerController = FPSOwnerController == nullptr ? Cast<AFPSPlayerController>(FPSOwnerPlayer->Controller) : FPSOwnerController;
		if(FPSOwnerController)
		{
			FPSOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	FPSOwnerPlayer = FPSOwnerPlayer == nullptr ? Cast<AFPSCharacter>(GetOwner()) : FPSOwnerPlayer;
	if(FPSOwnerPlayer && FPSOwnerPlayer->GetCombat() && IsFull())
	{
		FPSOwnerPlayer->GetCombat()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{
		FPSOwnerPlayer = nullptr;
		FPSOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}


void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch(WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			if (WeaponType == EWeaponType::EWT_SubmachineGun)
			{
				WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				WeaponMesh->SetEnableGravity(true);
				WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
			}
			break;
		case EWeaponState::EWS_Dropped:
			if(HasAuthority())
			{
				AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
			break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch(WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		}
		break;

	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if(BulletCasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());
		
			if(BulletCasingClass)
			{
				UWorld* World = GetWorld();
				if(World)
				{
					World->SpawnActor<ABulletCasing>(
						BulletCasingClass,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()
					);
				}
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr); // player is dead, so shouldn't be the owner any longer
	FPSOwnerPlayer = nullptr;
	FPSOwnerController = nullptr;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}
