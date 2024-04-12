// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "OmniKartHud.generated.h"


UCLASS(config = Game)
class AOmniKartHud : public AHUD
{
	GENERATED_BODY()

public:
	AOmniKartHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
