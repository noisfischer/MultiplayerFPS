// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletCasing.h"

ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletCasingMesh"));
	SetRootComponent(CasingMesh);
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();
	
}
