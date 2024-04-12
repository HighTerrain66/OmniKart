// Copyright Epic Games, Inc. All Rights Reserved.

#include "OmniKartGameMode.h"
#include "GoKart.h"
#include "OmniKartHud.h"

AOmniKartGameMode::AOmniKartGameMode()
{
	DefaultPawnClass = AGoKart::StaticClass();
	HUDClass = AOmniKartHud::StaticClass();
}
