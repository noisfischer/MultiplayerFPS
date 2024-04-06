// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletCasing.h"

ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletCasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// PHYSICS
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();
	
}
