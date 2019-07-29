// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "PoopGameMode.h"
#include "PoopHUD.h"
#include "PoopCharacter.h"
#include "UObject/ConstructorHelpers.h"

APoopGameMode::APoopGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = APoopHUD::StaticClass();
}
