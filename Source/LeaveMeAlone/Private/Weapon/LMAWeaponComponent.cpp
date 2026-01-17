// LeaveMeAlone Game by Netologiya. All RightsReserved.


#include "Weapon/LMAWeaponComponent.h"
#include "GameFramework/Character.h"
#include "Weapon/LMABaseWeapon.h"
#include "Animations/LMAReloadFinishedAnimNotify.h"


ULMAWeaponComponent::ULMAWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void ULMAWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnWeapon();
	InitAnimNotify();

	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Subscribing to delegates for weapon: %s"), *Weapon->GetName());

	if (Weapon)
	{
		if (!Weapon->OnAmmoDepleted.IsBound())
		{
			Weapon->OnAmmoDepleted.AddDynamic(this, &ULMAWeaponComponent::OnAmmoDepletedCallback);
			UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Subscribed to OnAmmoDepleted."));
		}

		if (!Weapon->OnClipFull.IsBound())
		{
			Weapon->OnClipFull.AddDynamic(this, &ULMAWeaponComponent::OnClipFullCallback);
			UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Subscribed to OnClipFull."));
		}
	}
}


void ULMAWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void ULMAWeaponComponent::SpawnWeapon()
{
	if (WeaponClass)
	{
		Weapon = GetWorld()->SpawnActor<ALMABaseWeapon>(WeaponClass);
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Spawned weapon: %s"), Weapon ? *Weapon->GetName() : TEXT("nullptr"));
		if (Weapon)
		{
			const auto Character = Cast<ACharacter>(GetOwner());
			if (Character)
			{
				FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
				Weapon->AttachToComponent(Character->GetMesh(), AttachmentRules, "r_Weapon_Socket");
			}
		}
	}
}

void ULMAWeaponComponent::Fire()
{
	if (Weapon && !AnimReloading)    
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			Character->StopAnimMontage(ReloadMontage);
		}
		AnimReloading = false;
	}

	if (Weapon && !AnimReloading)
	{
		Weapon->Fire();
	}
}

void ULMAWeaponComponent::InitAnimNotify()
{
	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: InitAnimNotify called."));

	if (!ReloadMontage) return;

	const auto NotifiesEvents = ReloadMontage->Notifies;
	for (auto NotifyEvent : NotifiesEvents)
	{
		auto ReloadFinish = Cast<ULMAReloadFinishedAnimNotify>(NotifyEvent.Notify);
		if (ReloadFinish)
		{
			ReloadFinish->OnNotifyReloadFinished.AddUObject(this, &ULMAWeaponComponent::OnNotifyReloadFinished);
			break;
		}
	}
}

void ULMAWeaponComponent::OnNotifyReloadFinished(USkeletalMeshComponent* SkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: OnNotifyReloadFinished triggered."));
	const auto Character = Cast<ACharacter>(GetOwner());
	if (Character && Character->GetMesh() == SkeletalMesh)
	{
		if (Weapon)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Calling Weapon->ChangeClip()."));
			Weapon->ChangeClip(); 
		}
		AnimReloading = false;             
	}
}

bool ULMAWeaponComponent::CanReload() const
{
	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: CanReload() called."));

	if (AnimReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: AnimReloading is true."));
		return false;
	}

	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Weapon is null."));
		return false;
	}

	if (IsClipFull())
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Clip is full."));
		return false;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: CanReload() returned true."));
	return true;
}

void ULMAWeaponComponent::AutoReload()
{
	if (CanReload())
	{
		PerformReload();
	}
}

void ULMAWeaponComponent::PerformReload()
{
	if (!Weapon || !ReloadMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: PerformReload() - Weapon or ReloadMontage is null."));
		AnimReloading = false;
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: PerformReload() - Character is null."));
		AnimReloading = false;
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: Performing reload animation."));
	AnimReloading = true;
	Character->PlayAnimMontage(ReloadMontage);
}

bool ULMAWeaponComponent::IsClipFull() const
{
	if (!Weapon)
		return false;

	int32 CurrentBullets = Weapon->GetCurrentClipBullets();
	int32 MaxBullets = Weapon->GetMaxClipBullets();

	return CurrentBullets >= MaxBullets;
}

void ULMAWeaponComponent::OnAmmoDepletedCallback(USkeletalMeshComponent* SkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: OnAmmoDepletedCallback triggered!"));
	if (CanReload())
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: CanReload() returned true. Calling Reload()."));
		AutoReload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ULMAWeaponComponent: CanReload() returned false."));
	}
}

void ULMAWeaponComponent::OnClipFullCallback(USkeletalMeshComponent* SkeletalMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("Clip full callback triggered."));
}

bool ULMAWeaponComponent::GetCurrentWeaponAmmo(FAmmoWeapon& AmmoWeapon) const
{
	if (!Weapon)
	{
		return false;
	}

	AmmoWeapon = Weapon->GetAmmoWeapon();

	return true;
}