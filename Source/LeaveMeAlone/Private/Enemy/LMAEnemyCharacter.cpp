// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "Enemy/LMAEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


ALMAEnemyCharacter::ALMAEnemyCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<ULMAHealthComponent>(TEXT("HealthComponent"));
}

void ALMAEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALMAEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}