// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
