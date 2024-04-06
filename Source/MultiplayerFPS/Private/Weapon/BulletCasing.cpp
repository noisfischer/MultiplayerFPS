// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BulletCasing.h"

// Sets default values
ABulletCasing::ABulletCasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABulletCasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

