// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Weapon/Weapon.h"
#include "Character/FPSCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/FPSPlayerController.h"
#include "HUD/PlayerHUD.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 300.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register replicated variables
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(PlayerRef)
	{
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if(PlayerRef->GetFollowCamera())
		{
			DefaultFOV = PlayerRef->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if(PlayerRef->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if(PlayerRef && PlayerRef->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		
		InterpFOV(DeltaTime); // for aiming
	}
	
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(PlayerRef == nullptr || PlayerRef->Controller == nullptr)
	{
		return;
	}
	// Should only use the Cast once, once this component is created
	PlayerController = PlayerController == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : PlayerController;

	if(PlayerController)
	{
		HUD = HUD == nullptr ? Cast<APlayerHUD>(PlayerController->GetHUD()) : HUD;
		if(HUD)
		{
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			
			// Calculate Crosshair Spread
			
			// MOVEMENT BASED SPREAD 
			FVector2D WalkSpeedRange(0.f, PlayerRef->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = PlayerRef->GetVelocity();
			Velocity.Z = 0.f;
			
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// FALLING BASED SPREAD - additive
			if(PlayerRef->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, .5f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 30.f);

			HUDPackage.CrosshairSpread = // .5f for initial spread
				.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}

	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			EquippedWeapon->GetZoomedFov(),
			DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed()
			);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV,
			DefaultFOV,
			DeltaTime,
			ZoomInterpSpeed
			);
	}
	if(PlayerRef && PlayerRef->GetFollowCamera())
	{
		PlayerRef->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr || PlayerRef == nullptr) return;
	PlayerRef->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireRate
		);
}

void UCombatComponent::FireTimerFinished()
{
	if(EquippedWeapon == nullptr) return;
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
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
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = PlayerRef->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, PlayerRef->GetMesh());
		}
		PlayerRef->GetCharacterMovement()->bOrientRotationToMovement = false;
		PlayerRef->bUseControllerRotationYaw = true;
		
		if(EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquipSound,
				PlayerRef->GetActorLocation()
			);
		}
	}
}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;

	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : PlayerController;
	if(PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		ServerFire(HitTarget); // calls Server RPC below
		// Sends a message to server to perform that function

		if(EquippedWeapon)
		{
			bCanFire = false;
			CrosshairShootingFactor = 2.f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireWeaponButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if(bFireButtonPressed && EquippedWeapon)
	{
		Fire();
	}
}

// Line trace will happen from center of viewport
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Retrieve World Coordinates of our screen center
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
		);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if(PlayerRef)	// prevent line trace from starting behind player
		{
			float DistanceToPlayer = (PlayerRef->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToPlayer + 100.f);
		}
		
		FVector End = Start + (CrosshairWorldDirection * TRACE_LENGTH); // Macro assigned in header

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
			// all other inputs are optional
			);

		if(TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

// Server RPC - invoked only on server
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultiCastFire(TraceHitTarget); // Applies to ALL clients and server
}

// Multicast RPC - happens to server and all clients - Our end result
void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}
	if(PlayerRef && CombatState == ECombatState::ECS_Unoccupied)
	{
		PlayerRef->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(PlayerRef == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = PlayerRef->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, PlayerRef->GetMesh());
	}

	EquippedWeapon->SetOwner(PlayerRef);
	EquippedWeapon->SetHUDAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	PlayerController = PlayerController == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : PlayerController;
	if(PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}

	if(EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			PlayerRef->GetActorLocation()
		);
	}

	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	
	// Character always faces forwards when holding weapon
	PlayerRef->GetCharacterMovement()->bOrientRotationToMovement = false;
	PlayerRef->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon)
	{
		ServerReload();
		HandleReload();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if(PlayerRef == nullptr || EquippedWeapon == nullptr) return;
	
	int32 ReloadAmount = AmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<AFPSPlayerController>(PlayerRef->Controller) : PlayerController;
	if(PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
		
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

// For server only
void UCombatComponent::ServerReload_Implementation()
{
	if(PlayerRef == nullptr || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_Reloading; // triggers OnRep_CombatState
	HandleReload(); // server sees reload montage
}

void UCombatComponent::FinishReloading()
{
	if(PlayerRef == nullptr) return;
	if(PlayerRef->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload(); // all clients see reload montage
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::HandleReload()
{
	if(PlayerRef)
	{
		PlayerRef->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}
