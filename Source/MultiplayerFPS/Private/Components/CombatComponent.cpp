// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "Character/FPSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 300.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register replicated variables
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(PlayerRef)
	{
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

// ON SERVER
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming =  bIsAiming; // so the client can aim before the server receives the RPC message
	ServerSetAiming(bIsAiming);
	if(PlayerRef)
	{
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

// ON CLIENT
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(PlayerRef)
	{
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && PlayerRef)
	{
		// Character always faces forwards when holding weapon
		PlayerRef->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerRef->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireWeaponButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; // happens on client - others don't need to know

	if(bFireButtonPressed)
	{
		ServerFire(); // calls Server RPC below
		// Sends a message to server to perform that function
	}
}

// Server RPC - invoked only on server
void UCombatComponent::ServerFire_Implementation()
{
	MultiCastFire(); // Applies to ALL clients and server
}

// Multicast RPC - happens to server and all clients - Our end result
void UCombatComponent::MultiCastFire_Implementation()
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}
	if(PlayerRef)
	{
		PlayerRef->PlayFireMontage(bAiming);
		EquippedWeapon->Fire();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(PlayerRef == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = PlayerRef->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PlayerRef->GetMesh());
	}

	EquippedWeapon->SetOwner(PlayerRef);
	
	// Character always faces forwards when holding weapon
	PlayerRef->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerRef->bUseControllerRotationYaw = true;
}
