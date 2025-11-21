// LeaveMeAlone Game by Netologiya. All RightsReserved.

#include "Player/LMADefaultCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


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
}

void ALMADefaultCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	FHitResult ResultHit;
	PC->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, ResultHit);

	if (CurrentCursor)
	{
		CurrentCursor->SetWorldLocation(ResultHit.Location);

		float Yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ResultHit.Location).Yaw;
		CurrentCursor->SetWorldRotation(FRotator(0.0f, Yaw, 0.0f));
	}
}

void ALMADefaultCharacter::Tick(float DeltaTime )
{
	Super::Tick(DeltaTime);

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

			FRotator NewControllerRot = FMath::RInterpTo(CurrentControllerRot, LookAtRot, DeltaTime, 5.0f);

			PC->SetControlRotation(NewControllerRot);
		}
	}
}

void ALMADefaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::MoveRight);
		EnhancedInputComponent->BindAction(IA_ZoomCameraAction, ETriggerEvent::Triggered, this, &ALMADefaultCharacter::ZoomCamera);
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