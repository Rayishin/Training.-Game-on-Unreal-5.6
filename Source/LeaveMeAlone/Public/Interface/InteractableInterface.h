// LeaveMeAlone Game by Netologiya. All RightsReserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"


UINTERFACE(Blueprintable)
class LEAVEMEALONE_API UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class LEAVEMEALONE_API IInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Instigator);
};