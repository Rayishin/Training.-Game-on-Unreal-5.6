// LeaveMeAlone Game by Netologiya. All RightsReserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "LMADefaultCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class LEAVEMEALONE_API ALMADefaultCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ALMADefaultCharacter();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* IMC_PlayerMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_MoveForward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_MoveRight;

	UPROPERTY()
	UDecalComponent* CurrentCursor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
	float MinZoomDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom", meta = (ClampMin = "50.0", ClampMax = "3000.0"))
	float MaxZoomDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float ZoomSpeed = 1.0f;

	float TargetArmLength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_ZoomCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 600.0f;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void MoveForward(const FInputActionValue& Value);
	virtual void MoveRight(const FInputActionValue& Value);
	virtual void ZoomCamera(const FInputActionValue& Value);

private:
	float YRotation = -75.0f;
	float ArmLength = 1400.0f;
	float FOV = 55.0f;
};
