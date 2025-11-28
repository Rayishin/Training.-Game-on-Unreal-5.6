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
class ULMAHealthComponent;
class UAnimMontage;


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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Cursor")
    class UDecalComponent* CurrentCursor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	class UMaterialInterface* CursorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components|Health")
	ULMAHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadWrite, Category = "Sprint")
	float Stamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaRegenRate = 10.0f; 

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaDrainRate = 20.0f; 

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float BaseMoveSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintMoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = true))
	UAnimMontage* SprintMontage;

	UPROPERTY(BlueprintReadWrite, Category = "Sprint")
	bool bIsSprinting = false;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void MoveForward(const FInputActionValue& Value);
	virtual void MoveRight(const FInputActionValue& Value);
	virtual void ZoomCamera(const FInputActionValue& Value);

	UFUNCTION()
	ULMAHealthComponent* GetHealthComponent() const { return HealthComponent; }

	void StartSprint();
	void StopSprint();

	bool IsSprinting() const { return bIsSprinting; }
	float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetIsSprintingInAnimation(bool bNewSprintingValue);

private:
	float YRotation = -75.0f;
	float ArmLength = 1400.0f;
	float FOV = 55.0f;

	void OnDeath();

	void RotationPlayerOnCursor();

	void OnHealthChanged(float NewHealth);

	FInputActionValue OnSprintPressed;
	FInputActionValue OnSprintReleased;
};
