// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "LevelActors/Damage/LMADamageActor.h"
#include "Kismet/GameplayStatics.h"


ALMADamageActor::ALMADamageActor()
{
 	
	PrimaryActorTick.bCanEverTick = true;


	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(SphereRadius);
	SetRootComponent(SphereComponent);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(SphereComponent);
	StaticMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.05f));
	StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -4.0f));
}

void ALMADamageActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALMADamageActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), SphereRadius, nullptr, {}, this, nullptr, false);
}

