// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CharacterAnimInstance.h"
#include "Character/FPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// BeginPlay
void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerRef = Cast<AFPSCharacter>(TryGetPawnOwner());
}

// Tick
void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// in case BeginPlay PlayerRef cast is unsuccessful
	if(PlayerRef == nullptr)
	{
		PlayerRef = Cast<AFPSCharacter>(TryGetPawnOwner());
	}
	if(PlayerRef == nullptr)
	{
		return;
	}

	// Set character speed
	FVector Velocity = PlayerRef->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = PlayerRef->GetCharacterMovement()->IsFalling();
	bIsAccelerating = PlayerRef->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = PlayerRef->IsWeaponEquipped();
	bIsCrouched = PlayerRef->bIsCrouched; // bIsCrouched is pre-existing in ACharacter
	bAiming = PlayerRef->IsAiming();

	// Offset Yaw for Strafing
	FRotator AimRotation = PlayerRef->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerRef->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 15.f);
	YawOffset = DeltaRotation.Yaw;

	PlayerRotationLastFrame = PlayerRotation;
	PlayerRotation = PlayerRef->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(PlayerRotation, PlayerRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
}
