// Shooter, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterCoreTypes.h"
#include "ShooterWeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API UShooterWeaponComponent : public UActorComponent
{

	GENERATED_BODY()

public:	

	UShooterWeaponComponent(
	);

	void StartShooting(
	);

	void StopShooting(
	);

	void EquipNextWeapon(
	);

	void ReloadWeapon(
	);

	bool GetCurrentWeaponUIData(
		FWeaponUIData &UIData
	) const;

	bool GetCurrentWeaponAmmoData(
		FAmmoData &AmmoData
	) const;

protected:

	virtual void BeginPlay(
	) override;

	virtual void EndPlay(
		EEndPlayReason::Type const EndPlayReason
	) override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<FWeaponData> WeaponData;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponEquippedSocketName = TEXT("WeaponSocket");

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponArmorySocketName = TEXT("ArmorySocket");

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage *EquipAnimMontage;

private:

	void SpawnWeapons(
	);

	void AttachWeaponToSocket(
		AShooterBaseWeapon *Weapon,
		USceneComponent *SceneComponent,
		FName const &SocketName
	);

	void EquipWeapon(
		int32 WeaponIndex
	);

	void PlayAnimMontage(
		UAnimMontage *Montage
	);

	USkeletalMeshComponent *GetOwnerSkeletalMesh(
	);

	void InitAnimations(
	);

	void InitEquipAnimation(
	);

	void InitReloadAnimation(
		UAnimMontage *ReloadMontage
	);

	bool CanShoot(
	) const;

	bool CanEquip(
	) const;

	bool CanReload(
	) const;

	void ChangeClip(
	);

	UFUNCTION()
	void OnEquipFinished(
		USkeletalMeshComponent *MeshComp
	);

	UFUNCTION()
	void OnReloadFinished(
		USkeletalMeshComponent *MeshComp
	);

	UFUNCTION()
	void OnEmptyClip(
	);

	UPROPERTY()
	TArray<AShooterBaseWeapon *> Weapons;

	UPROPERTY()
	AShooterBaseWeapon *CurrentWeapon = nullptr;

	UPROPERTY()
	UAnimMontage *CurrentReloadAnimMontage = nullptr;

	int32 CurrentWeaponIndex = 0;

	bool bActionAnimInProgress = false;
};
