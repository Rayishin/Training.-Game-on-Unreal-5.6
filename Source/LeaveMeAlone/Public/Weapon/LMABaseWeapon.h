// LeaveMeAlone Game by Netologiya. All RightsReserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LMABaseWeapon.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoDepleted, class USkeletalMeshComponent*, SkeletalMesh);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClipFull, class USkeletalMeshComponent*, SkeletalMesh);

USTRUCT(BlueprintType)
struct FAmmoWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 Bullets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	int32 Clips;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	bool Infinite;
};

class USkeletalMeshComponent;

UCLASS()
class LEAVEMEALONE_API ALMABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ALMABaseWeapon();

	void Fire();
	void Shoot();

	void DecrementBullets();
	bool IsCurrentClipEmpty() const;
	void ChangeClip();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	int32 GetCurrentClipBullets() const { return CurrentClipBullets; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	int32 GetMaxClipBullets() const { return MaxClipBullets; }

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoDepleted OnAmmoDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnClipFull OnClipFull;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	USkeletalMeshComponent* WeaponComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	float TraceDistance = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	FAmmoWeapon AmmoWeapon{ 30, 5, false };

	int32 GetBullets() const { return AmmoWeapon.Bullets; }
	int32 GetClips() const { return AmmoWeapon.Clips; }
	bool IsInfinite() const { return AmmoWeapon.Infinite; }

protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	int32 CurrentClipBullets;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	int32 MaxClipBullets;

public:	
	
	virtual void Tick(float DeltaTime) override;
};