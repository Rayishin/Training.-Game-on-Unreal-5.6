// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "LevelActors/Pickup/LMAHealthPickup.h"
#include "Components/LMAHealthComponent.h"


ALMAHealthPickup::ALMAHealthPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SetRootComponent(SphereComponent);
}

void ALMAHealthPickup::BeginPlay()
{
	Super::BeginPlay();
}

void ALMAHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ALMAHealthPickup::GivePickup(ALMADefaultCharacter* Character)
{
	if (!Character) return false;

	const auto HealthComponent = Character->GetHealthComponent();
	if (!HealthComponent) return false;

	return HealthComponent->AddHealth(HealthFromPickup);
}

void ALMAHealthPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bIsTaken) return;

	if (ALMADefaultCharacter* Player = Cast<ALMADefaultCharacter>(OtherActor))
	{
		if (GivePickup(Player))
		{
			bIsTaken = true;
			Destroy();
		}
	}
}