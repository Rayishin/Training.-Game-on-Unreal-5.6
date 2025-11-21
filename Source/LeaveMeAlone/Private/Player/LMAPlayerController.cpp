// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "Player/LMAPlayerController.h"
#include "Player/LMADefaultCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


ALMAPlayerController::ALMAPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCRef(TEXT("/Game/Input/IMC_PlayerMovement"));
	if (IMCRef.Succeeded())
	{
		IMC_PlayerMovement = IMCRef.Object;
	}
}

void ALMAPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;

	EnableInput(this);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (IMC_PlayerMovement)
		{
			Subsystem->AddMappingContext(IMC_PlayerMovement, 0);
		}
	}
}