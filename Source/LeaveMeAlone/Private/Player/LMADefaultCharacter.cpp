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
#include "LMAGameMode.h"


ALMADefaultCharacter::ALMADefaultCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SetRelativeRotation(FRotator(-25.0f, 0.0f, 0.0f));
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;      
	CameraBoom->bEnableCameraLag = true;       
	CameraBoom->CameraLagSpeed = 20.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 90.0f;

	HealthComponent = CreateDefaultSubobject<ULMAHealthComponent>(TEXT("HealthComponent"));

	if (UCharacterMovementComponent* CharMove = GetCharacterMovement())
	{
		CharMove->bOrientRotationToMovement = false;
	}

	TargetArmLength = CameraBoom->TargetArmLength;

	static ConstructorHelpers::FObjectFinder<UInputAction> ZoomCameraRef(TEXT("/Game/Input/IA_ZoomCamera"));
	if (ZoomCameraRef.Succeeded())
	{
		IA_ZoomCameraAction = ZoomCameraRef.Object;
	}

	WeaponComponent = CreateDefaultSubobject<ULMAWeaponComponent>(TEXT("WeaponComponent"));
}

void ALMADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUObject(this, &ALMADefaultCharacter::OnDeath);
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
		EnhancedInputComponent->BindAction(IA_Turn, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::Turn);
		EnhancedInputComponent->BindAction(IA_LookUp, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::LookUp);
		EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Completed, this, &ALMADefaultCharacter::Interact);

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

		if (CameraBoom)
		{
			CameraBoom->TargetArmLength = TargetArmLength;
		}
	}
}

void ALMADefaultCharacter::OnDeath()
{
	if (DeathMontage && GetMesh())
	{
		GetMesh()->PlayAnimation(DeathMontage, 1.0f);
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		PC->SetViewTargetWithBlend(this, 3.5f);
	}

	if (Controller)
	{
		Controller->SetIgnoreMoveInput(true);
	}

	if (CameraBoom)
	{
		CameraBoom->bUsePawnControlRotation = false;
		float CharacterHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f;
		CameraBoom->TargetArmLength = CharacterHeight * 3.0f;
		CameraBoom->bEnableCameraLag = true;  
		CameraBoom->CameraLagSpeed = 3.0f;
	}

	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.0f);

	OnDeath_BP();
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
	if (bIsSprinting)  
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fire while sprinting!"));
		return;  
	}

	if (WeaponComponent && !WeaponComponent->AnimReloading)
	{
		WeaponComponent->Fire();
	}
}

void ALMADefaultCharacter::Turn(const FInputActionValue& Value)
{
	AddControllerYawInput(Value.Get<float>() * MouseSensitivity);
}

void ALMADefaultCharacter::LookUp(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value.Get<float>() * MouseSensitivity);
}

void ALMADefaultCharacter::Interact(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("=== Interaction is pressed ==="));

	APlayerController* PC = GetController<APlayerController>();
	if (!PC) return;

	FVector Start = PC->GetPawn()->GetActorLocation();
	FVector End = Start + (PC->GetControlRotation().Vector() * 500.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace got into: %s"), *Hit.GetActor()->GetName());

		if (Hit.GetActor())
		{
			if (Hit.GetActor()->Implements<UInteractableInterface>())
			{
				UE_LOG(LogTemp, Warning, TEXT("The actor implements the interface! Calling Interact!"));
				IInteractableInterface::Execute_Interact(Hit.GetActor(), this);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("The actor does NOT implement the interface!"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace I DIDN't hit ANYTHING! Increase the distance!"));
	}
}