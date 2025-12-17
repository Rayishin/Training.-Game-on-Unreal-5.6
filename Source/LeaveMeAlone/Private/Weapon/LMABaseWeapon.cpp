// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "Weapon/LMABaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/LMAWeaponComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


DEFINE_LOG_CATEGORY_STATIC(LogWeapon, All, All);

ALMABaseWeapon::ALMABaseWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	WeaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	SetRootComponent(WeaponComponent);

	CurrentClipBullets = AmmoWeapon.Bullets;
	MaxClipBullets = AmmoWeapon.Bullets;
}

void ALMABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogWeapon, Warning, TEXT("ALMABaseWeapon::BeginPlay() — LogWeapon is ACTIVE!"));
}

void ALMABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALMABaseWeapon::Fire()
{
	Shoot();
}

void ALMABaseWeapon::Shoot()
{
	UE_LOG(LogWeapon, Warning, TEXT("Shoot() called"));

	const FTransform SocketTransform = WeaponComponent->GetSocketTransform("Muzzle");
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ShootDirection = SocketTransform.GetRotation().GetForwardVector();
	const FVector TraceEnd = TraceStart + ShootDirection * TraceDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);
	FVector TracerEnd = TraceEnd;

	if (HitResult.bBlockingHit)
	{
		TracerEnd = HitResult.ImpactPoint;
	}

	SpawnTrace(TraceStart, TracerEnd);
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShootWave, TraceStart);

	DecrementBullets();
}

void ALMABaseWeapon::ChangeClip()
{
	if (AmmoWeapon.Clips > 0)
	{
		CurrentClipBullets = MaxClipBullets;
		AmmoWeapon.Clips--;

		AmmoWeapon.Bullets = CurrentClipBullets;
		OnClipFull.Broadcast(WeaponComponent);
	}
}

bool ALMABaseWeapon::IsCurrentClipEmpty() const
{
	return CurrentClipBullets <= 0;
}

void ALMABaseWeapon::DecrementBullets()
{
	if (AmmoWeapon.Infinite) return;

	if (CurrentClipBullets <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ALMABaseWeapon: Mag is already empty!"));
		return;
	}

	CurrentClipBullets--;
	AmmoWeapon.Bullets = CurrentClipBullets;
	UE_LOG(LogTemp, Warning, TEXT("ALMABaseWeapon: DecrementBullets. CurrentClipBullets: %d"), CurrentClipBullets);

	if (CurrentClipBullets <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ALMABaseWeapon: Mag is empty! Clips left: %d"), AmmoWeapon.Clips);
		OnAmmoDepleted.Broadcast(WeaponComponent); 
		
		if (AmmoWeapon.Clips > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("ALMABaseWeapon: Auto-reloading..."));

			ULMAWeaponComponent* WeaponComp = Cast<ULMAWeaponComponent>(GetOwner());
			if (WeaponComp)
			{
				WeaponComp->AutoReload(); 
			}
		}
	}
}

void ALMABaseWeapon::SpawnTrace(const FVector& TraceStart, const FVector& TraceEnd)
{
	const auto TraceFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TraceEffect, TraceStart);

	if (TraceFX)
	{
		TraceFX->SetNiagaraVariableVec3(TraceName, TraceEnd);
	}
}