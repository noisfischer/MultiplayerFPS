// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "MultiplayerFPS/FPSTypes/TurningInPlace.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Interfaces/RagdollInterface.h"
#include "MultiplayerFPS/FPSTypes/CombatState.h"
#include "FPSCharacter.generated.h"

UCLASS()
class MULTIPLAYERFPS_API AFPSCharacter : public ACharacter, public IInteractWithCrosshairsInterface, public IRagdollInterface
{
	GENERATED_BODY()

public:
	AFPSCharacter();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();

	void Elim();
	
	// RPC - happens on all clients
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	virtual void Destroyed() override;

	// RagdollInterface function
	virtual void GetRagdollInfo_Implementation(const FName& BoneName, const FVector& ImpulseDirection) override;
	
	// Inherited from AActor class
	virtual void OnRep_ReplicatedMovement() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY()
	class AFPSPlayerState* FPSPlayerState;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	UPROPERTY(EditAnywhere)
	float AimSensitivityMultiplier = 1.f;
	
protected:
	virtual void BeginPlay() override;
	void RotateInPlace(float DeltaTime);

	void Move(const struct FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void CrouchButtonReleased();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireWeaponButtonPressed();
	void FireWeaponButtonReleased();
	void ReloadButtonPressed();
	void CalculateAO_Pitch();
	float CalculateSpeed();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void PlayHitReactMontage();
	
	// callback for pre-existing ApplyDamage function
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();
	void PollInit(); // Poll for any relevant classes and init our HUD

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	// RepNotify function
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); // ONLY CALLED ON THE CLIENT

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	
	// Remote Procedure Call
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw; // for left/right aim offset
	float InterpAO_Yaw;
	float AO_Pitch; // for up/down aim offset
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/*
	 * Animation Montages
	 */
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	/*
	 * Player Health
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = MaxHealth;

	UFUNCTION() // when CurrentHealth gets updated, this is called
	void OnRep_Health();

	UPROPERTY()
	class AFPSPlayerController* FPSPlayerController;

	// Ragdoll on elim information
	UPROPERTY(Replicated)
	FName LastHitBone;
	UPROPERTY(Replicated)
	FVector RagdollDirection;

	// Elimination members
	bool bElimmed = false;
	FTimerHandle ElimTimer;
	void ElimTimerFinished();
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	/*
	 * Dissolve Effect
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack; // delegate bound in StartDissolve

	UPROPERTY(EditAnywhere) // update in BP
	UCurveFloat* DissolveCurve;
	
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1;
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance2;

	// Material instance set in BP used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance1; // for top of mesh
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance2; // for bottom of mesh

	bool bInputsSet = false;
	
public:	
	void SetOverlappingWeapon(AWeapon* Weapon); // ONLY CALLED ON THE SERVER
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
};
