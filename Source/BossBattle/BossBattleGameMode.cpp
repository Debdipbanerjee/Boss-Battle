// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossBattleGameMode.h"
#include "BossBattleCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABossBattleGameMode::ABossBattleGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
