// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CharacterAnimInstance.h"
#include "Character/FPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerRef = Cast<AFPSCharacter>(TryGetPawnOwner());
}

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
}
