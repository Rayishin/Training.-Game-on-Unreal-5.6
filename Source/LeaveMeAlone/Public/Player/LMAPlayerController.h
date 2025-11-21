// LeaveMeAlone Game by Netologiya. All RightsReserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "LMAPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LEAVEMEALONE_API ALMAPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ALMAPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* IMC_PlayerMovement;
};
