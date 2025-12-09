// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "Weapon/LMABaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapon/LMAWeaponComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"


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

	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Black, false, 1.0f, 0, 2.0f);
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

	if (HitResult.bBlockingHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 5.0f, 24, FColor::Red, false, 1.0f);
	}

	DecrementBullets();
}

void ALMABaseWeapon::ChangeClip()
{
	if (AmmoWeapon.Clips > 0)
	{
		CurrentClipBullets = MaxClipBullets;
		AmmoWeapon.Clips--;

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