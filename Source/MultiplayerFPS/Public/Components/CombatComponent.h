// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/PlayerHUD.h"
#include "Weapon/WeaponTypes.h"
#include "MultiplayerFPS/FPSTypes/CombatState.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERFPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AFPSCharacter; // allows access to its variables and functions
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireWeaponButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();
	
protected:
	virtual void BeginPlay() override;
	
	void SetAiming(bool bIsAiming);
	
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming); // RPC! It can have input parameters

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();
	
	UFUNCTION(Server, Reliable)	// Server RPC - happens only on server
	void ServerFire(const FVector_NetQuantize& TraceHitTarget); // NetQuantize rounds vectors to whole numbers (helps lower bandwidth)

	UFUNCTION(NetMulticast, Reliable)	// Multicast RPC - invoked on server and all clients
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

private:
	UPROPERTY()
	class AFPSCharacter* PlayerRef;
	UPROPERTY()
	class AFPSPlayerController* PlayerController;
	UPROPERTY()
	class APlayerHUD* HUD;

	// replicated to all clients (no repnotify)
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
	 * HUD and Crosshairs
	 */
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FHUDPackage HUDPackage;
	
	FVector HitTarget;

	/*
	 * Aiming and FOV
	 */
	
	float DefaultFOV; // FOV when not aiming, set to camera's base FOV in BeginPlay
	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	 * AUTOMATIC FIRE
	 */
	FTimerHandle FireTimer;

	bool bCanFire = true;
	
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// ammo for currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
	
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
public:	
	
};
