// LeaveMeAlone Game by Netologiya. All RightsReserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Player/LMADefaultCharacter.h"
#include "LMAHealthPickup.generated.h"


UCLASS()
class LEAVEMEALONE_API ALMAHealthPickup : public AActor
{
	GENERATED_BODY()
	
public:	

	ALMAHealthPickup();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (ClampMin = 5.0f, ClampMax = 100.0f))
	float HealthFromPickup = 100.0f;

	UPROPERTY()
	bool bIsTaken = false;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

public:	

	virtual void Tick(float DeltaTime) override;

private:

	bool GivePickup(ALMADefaultCharacter* Character);
};
