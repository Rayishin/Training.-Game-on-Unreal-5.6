// LeaveMeAlone Game by Netologiya. All RightsReserved.

#include "Player/LMADefaultCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/LMAHealthComponent.h"
#include "GameFramework/PlayerController.h"
#include "Animation/AnimInstanceProxy.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/LMAWeaponComponent.h"
#include "Weapon/LMABaseWeapon.h"


ALMADefaultCharacter::ALMADefaultCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->TargetArmLength = ArmLength;
	SpringArmComponent->SetRelativeRotation(FRotator(YRotation, 0.0f, 0.0f));
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->bEnableCameraLag = false;
	SpringArmComponent->bEnableCameraRotationLag = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetFieldOfView(FOV);
	CameraComponent->bUsePawnControlRotation = false;

	HealthComponent = CreateDefaultSubobject<ULMAHealthComponent>(TEXT("HealthComponent"));

	if (UCharacterMovementComponent* CharMove = GetCharacterMovement())
	{
		CharMove->bOrientRotationToMovement = false;
	}

	TargetArmLength = SpringArmComponent->TargetArmLength;

	static ConstructorHelpers::FObjectFinder<UInputAction> ZoomCameraRef(TEXT("/Game/Input/IA_ZoomCamera"));
	if (ZoomCameraRef.Succeeded())
	{
		IA_ZoomCameraAction = ZoomCameraRef.Object;
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	CurrentCursor = CreateDefaultSubobject<UDecalComponent>(TEXT("CurrentCursor"));
	if (CurrentCursor)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentCursor created successfully."));

		CurrentCursor->SetupAttachment(RootComponent);
		CurrentCursor->DecalSize = FVector(50.0f, 50.0f, 50.0f);
		CurrentCursor->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
		CurrentCursor->SetVisibility(false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create CurrentCursor!"));
	}

	WeaponComponent = CreateDefaultSubobject<ULMAWeaponComponent>(TEXT("WeaponComponent"));
}

void ALMADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();


	if (CursorMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cursor material is assigned."));

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(CursorMaterial, this);
		if (DynamicMaterial)
		{
			CurrentCursor->SetMaterial(0, DynamicMaterial);
			UE_LOG(LogTemp, Warning, TEXT("Dynamic Material applied to CurrentCursor."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Dynamic Material!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CursorMaterial is not assigned in editor!"));
	}

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUObject(this, &ALMADefaultCharacter::OnDeath);

		OnHealthChanged(HealthComponent->GetHealth());
		HealthComponent->OnHealthChanged.AddUObject(this, &ALMADefaultCharacter::OnHealthChanged);
	}


	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_PlayerMovement, 0);
		}
	}
}

void ALMADefaultCharacter::Tick(float DeltaTime )
{
	Super::Tick(DeltaTime);


	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FHitResult ResultHit;
	bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, true, ResultHit);

	UE_LOG(LogTemp, Warning, TEXT("Hit under cursor: %s"), bHit ? TEXT("Yes") : TEXT("No"));

	if (!bHit || !CurrentCursor)
	{
		if (CurrentCursor)
		{
			CurrentCursor->SetVisibility(false);
		}
			return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Hit location: %s"), *ResultHit.Location.ToString());

	CurrentCursor->SetVisibility(true);
	FVector HitLocation = ResultHit.Location; 
	CurrentCursor->SetWorldLocation(HitLocation);

	// FRotator DecalRotator = ResultHit.Normal.Rotation();
	// DecalRotator.Pitch += 90.f;
	// CurrentCursor->SetWorldRotation(DecalRotator.Quaternion());

	// UE_LOG(LogTemp, Warning, TEXT("Decal rotation: %s"), *DecalRotator.ToString());

	FVector HitNormal = ResultHit.Normal;
	FVector TargetForward = HitNormal;
	FVector TargetUp = FVector(0, 0, 1);
	FQuat DecalRotation = FQuat::FindBetweenVectors(FVector(1, 0, 0), TargetForward);
	CurrentCursor->SetWorldRotation(DecalRotation);

	FVector Direction = ResultHit.Location - GetActorLocation();
	Direction.Z = 0.0f;

	if (!Direction.IsNearlyZero())
	{
			FRotator LookAtRot = FRotationMatrix::MakeFromX(Direction).Rotator();
			LookAtRot.Pitch = 0.0f;
			LookAtRot.Roll = 0.0f;

			FRotator CurrentControllerRot = PC->GetControlRotation();
			FRotator NewControllerRot = FMath::RInterpTo(CurrentControllerRot, LookAtRot, GetWorld()->GetDeltaSeconds(), 5.0f);
			PC->SetControlRotation(NewControllerRot);
	}

	if (!(HealthComponent && HealthComponent->IsDead()))
	{
		RotationPlayerOnCursor();
	}

	UpdateStamina(DeltaTime);
}

void ALMADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent is valid"));

		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::MoveRight);
		EnhancedInputComponent->BindAction(IA_ZoomCameraAction, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::ZoomCamera);
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::Reload);

		if (IA_Sprint)
		{
			UE_LOG(LogTemp, Warning, TEXT("IA_Sprint is valid"));
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &ALMADefaultCharacter::StartSprint);
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &ALMADefaultCharacter::StopSprint);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("IA_Sprint is null!"));
		}
		
		if (IA_Fire)
		{
			EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Started, this, &ALMADefaultCharacter::OnStartFire);
			EnhancedInputComponent->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ALMADefaultCharacter::OnStopFire);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent is null!"));
	}
}

void ALMADefaultCharacter::MoveForward(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	AddMovementInput(GetActorForwardVector(), AxisValue * MoveSpeed);
}

void ALMADefaultCharacter::MoveRight(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();
	AddMovementInput(GetActorRightVector(), AxisValue * MoveSpeed);
}

void ALMADefaultCharacter::ZoomCamera(const FInputActionValue& Value)
{
	float AxisValue = Value.Get<float>();

	if (FMath::Abs(AxisValue) > 0.01f)
	{
		TargetArmLength -= AxisValue * ZoomSpeed * 15.0f;
		TargetArmLength = FMath::Clamp(TargetArmLength, MinZoomDistance, MaxZoomDistance);

		if (SpringArmComponent)
		{
			SpringArmComponent->TargetArmLength = TargetArmLength;
		}
	}
}

void ALMADefaultCharacter::OnDeath()
{
	if (CurrentCursor)
	{
		CurrentCursor->DestroyRenderState_Concurrent();
	}

	PlayAnimMontage(DeathMontage);
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.0f);

	if (Controller)
	{
		Controller->ChangeState(NAME_Spectating);
	}
}

void ALMADefaultCharacter::RotationPlayerOnCursor()
{

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FHitResult ResultHit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, true, ResultHit))
	{

		FVector Direction = ResultHit.Location - GetActorLocation();
		Direction.Z = 0.0f;

		if (!Direction.IsNearlyZero())
		{
			FRotator LookAtRot = FRotationMatrix::MakeFromX(Direction).Rotator();
			LookAtRot.Pitch = 0.0f;
			LookAtRot.Roll = 0.0f;

			FRotator CurrentControllerRot = PC->GetControlRotation();
			FRotator NewControllerRot = FMath::RInterpTo(CurrentControllerRot, LookAtRot, GetWorld()->GetDeltaSeconds(), 5.0f);
			PC->SetControlRotation(NewControllerRot);
		}
	}
}

void ALMADefaultCharacter::OnHealthChanged(float NewHealth)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FString::Printf(TEXT("Health = %f"), NewHealth));
}

void ALMADefaultCharacter::StartSprint()
{
	UE_LOG(LogTemp, Warning, TEXT("StartSprint called!"));

	if (Stamina <= 0.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartSprint: Stamina depleted, cannot sprint"));
		return;
	}

	bIsSprinting = true;
	UpdateMovementSpeed();

	UE_LOG(LogTemp, Warning, TEXT(">>> SPRINT ACTIVATED! bIsSprinting = TRUE <<<"));
}

void ALMADefaultCharacter::StopSprint()
{
	UE_LOG(LogTemp, Warning, TEXT("StopSprint called"));
	
	bIsSprinting = false;
	UpdateMovementSpeed();

	UE_LOG(LogTemp, Warning, TEXT("<<< SPRINT DEACTIVATED! bIsSprinting = FALSE >>>"));
}

void ALMADefaultCharacter::UpdateStamina(float DeltaTime)
{
	if(bIsSprinting)
	{
		Stamina -= StaminaDrainRate * DeltaTime;
		if (Stamina <= 0.0f)
		{
			Stamina = 0.0f;
			StopSprint(); 
		}
	}
	else
	{
		Stamina = FMath::Clamp(Stamina + StaminaRegenRate * DeltaTime, 0.0f, MaxStamina);
	}
}

void ALMADefaultCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALMADefaultCharacter, bIsSprinting);
}

void ALMADefaultCharacter::OnRep_IsSprinting()
{
	UpdateMovementSpeed();
}

void ALMADefaultCharacter::UpdateMovementSpeed()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintMoveSpeed : MoveSpeed;

	    UE_LOG(LogTemp, Warning, TEXT("MaxWalkSpeed set to %f"), GetCharacterMovement()->MaxWalkSpeed);
	}
}

void ALMADefaultCharacter::OnStartFire(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Fire button pressed."));
	if (WeaponComponent && !WeaponComponent->AnimReloading)
	{
		float FireRate = 0.1f;
		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ALMADefaultCharacter::TryFire, FireRate, true, 0.0f);
	}
}

void ALMADefaultCharacter::OnStopFire(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Fire button released."));
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	if (WeaponComponent && WeaponComponent->Weapon && WeaponComponent->Weapon->IsCurrentClipEmpty())
	{
		WeaponComponent->AutoReload();
	}
}

void ALMADefaultCharacter::Reload(const FInputActionValue& Value)
{
	if (bIsSprinting)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot reload while sprinting"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ALMADefaultCharacter: Reload button pressed."));
	if (WeaponComponent && !WeaponComponent->AnimReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("ALMADefaultCharacter: Calling WeaponComponent->Reload()."));
		WeaponComponent->AutoReload();
	}
}

void ALMADefaultCharacter::TryFire()
{
	if (WeaponComponent && !WeaponComponent->AnimReloading)
	{
		WeaponComponent->Fire();
	}
}