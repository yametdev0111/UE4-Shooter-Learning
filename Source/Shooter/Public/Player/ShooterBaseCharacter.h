// Shooter, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterBaseCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UShooterHealthComponent;
class UTextRenderComponent;

class AShooterBaseWeapon;
class UShooterWeaponComponent;


UCLASS()
class SHOOTER_API AShooterBaseCharacter : public ACharacter
{

	GENERATED_BODY()

public:

	AShooterBaseCharacter(
		FObjectInitializer const &ObjInitializer
	);

	virtual void Tick(
		float DeltaTime
	) override;

	virtual void SetupPlayerInputComponent(
		class UInputComponent *PlayerInputComponent
	) override;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead(
	) const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartRunning(
	);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopRunning(
	);

	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void StartShooting(
	);

	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void StopShooting(
	);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsRunning(
	) const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	float MoveDirectionRadians(
	) const;

	void SetPlayerColor(
		FLinearColor const &Color
	);

protected:

	virtual void BeginPlay(
	) override;

	virtual void EndPlay(
		EEndPlayReason::Type const EndPlayReason
	) override;

	virtual void OnDeath(
	);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent *SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent *CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	UShooterHealthComponent *HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Health")
	UTextRenderComponent *HealthTextComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	UShooterWeaponComponent *WeaponComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Movement(Fall)")
	FVector2D MinMaxDamagingFallVelocities = FVector2D(900.0f, 1800.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Movement(Fall)")
	FVector2D MinMaxFallDamage = FVector2D(10.0f, 100.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Lifespan", meta = (ClampMin = 0.0f))
	float LifeSpanAfterDeath = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	FName MaterialColorName = "Paint Color";

private:

	void MoveForward(
		float Scale
	);

	void MoveRight(
		float Scale
	);

	void OnHealthChanged(
		float NewHealth,
		float HealthDelta
	);

	UFUNCTION()
	void OnGroundLanding(
		FHitResult const &HitResult
	);


	bool bIsMovingForward = false;
	bool bIsRunning = false;
};
